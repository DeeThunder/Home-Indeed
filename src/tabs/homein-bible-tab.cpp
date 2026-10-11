#include "homein-bible-tab.hpp"
#include <QMessageBox>
#include <QRegularExpression>
#include "../detection/homein-ref-parser.hpp"

HomeInBibleTab::HomeInBibleTab(HomeInDB& db, QWidget *parent) : QWidget(parent), bible_db(db) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    // 1. Single Translation Selection & Browse Buttons
    QHBoxLayout* trans_layout = new QHBoxLayout();
    QLabel* trans_label = new QLabel("Translation:");
    translation_combo = new QComboBox();
    browse_books_btn = new QPushButton("📚 Books");
    browse_books_btn->setToolTip("Click to browse Bible books and chapters");

    trans_layout->addWidget(trans_label);
    trans_layout->addWidget(translation_combo, 1);
    trans_layout->addWidget(browse_books_btn);
    layout->addLayout(trans_layout);

    // 2. Search Bar Area
    QHBoxLayout* search_layout = new QHBoxLayout();
    search_box = new QLineEdit();
    search_box->setPlaceholderText("Search or click 🔍 to browse books...");
    search_box->setClearButtonEnabled(true);
    search_btn = new QPushButton("🔍");
    search_btn->setToolTip("Search or browse books");
    search_btn->setFixedWidth(36);

    search_layout->addWidget(search_box, 1);
    search_layout->addWidget(search_btn);
    layout->addLayout(search_layout);

    // 3. Navigation Status Label
    status_label = new QLabel("");
    status_label->setStyleSheet("font-weight: bold; color: #5294e2; padding: 2px 0;");
    status_label->setVisible(false);
    layout->addWidget(status_label);

    // 4. Interactive Book & Chapter Grid (Scrollable)
    grid_scroll = new QScrollArea();
    grid_scroll->setWidgetResizable(true);
    grid_scroll->setMinimumHeight(140);
    grid_scroll->setVisible(false);
    grid_container = new QWidget();
    grid_layout = new QGridLayout(grid_container);
    grid_layout->setSpacing(4);
    grid_layout->setContentsMargins(2, 2, 2, 2);
    grid_scroll->setWidget(grid_container);
    layout->addWidget(grid_scroll, 1);

    // 5. Verses Results List
    results_list = new QListWidget();
    results_list->setWordWrap(true);
    results_list->setMinimumHeight(80);
    layout->addWidget(results_list, 1);

    // 6. Action and Verse Step Buttons
    QHBoxLayout* action_layout = new QHBoxLayout();
    prev_verse_btn = new QPushButton("◀ Prev");
    prev_verse_btn->setToolTip("Preceding Verse in Chapter");
    next_verse_btn = new QPushButton("Next ▶");
    next_verse_btn->setToolTip("Succeeding Verse in Chapter");
    push_live_btn  = new QPushButton("Push Live");
    push_live_btn->setStyleSheet("font-weight: bold;");
    add_queue_btn  = new QPushButton("Add to Queue");

    action_layout->addWidget(prev_verse_btn);
    action_layout->addWidget(next_verse_btn);
    action_layout->addStretch();
    action_layout->addWidget(push_live_btn);
    action_layout->addWidget(add_queue_btn);
    layout->addLayout(action_layout);

    SetLiveState(false);

    // Populate translations
    PopulateTranslations();

    // Signal / Slot Connections
    connect(search_btn, &QPushButton::clicked, this, &HomeInBibleTab::OnSearchTriggered);
    connect(search_box, &QLineEdit::returnPressed, this, &HomeInBibleTab::OnSearchTriggered);
    connect(browse_books_btn, &QPushButton::clicked, this, &HomeInBibleTab::PopulateBookGrid);

    connect(prev_verse_btn, &QPushButton::clicked, this, &HomeInBibleTab::OnPrevVerseClicked);
    connect(next_verse_btn, &QPushButton::clicked, this, &HomeInBibleTab::OnNextVerseClicked);
    connect(push_live_btn,  &QPushButton::clicked, this, &HomeInBibleTab::OnPushLiveClicked);
    connect(add_queue_btn,  &QPushButton::clicked, this, &HomeInBibleTab::OnQueueClicked);
    connect(results_list,   &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item) {
            emit RequestPushToOverlay(item->data(Qt::UserRole).toString());
        }
    });

    connect(translation_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (!current_book.empty() && current_chapter > 0) {
            auto verses = bible_db.GetChapterVerses(current_book, current_chapter, CurrentTranslation());
            if (!verses.empty()) {
                int sel_row = results_list->currentRow();
                results_list->clear();
                for (const auto& v : verses) {
                    QString payload = QString("%1 %2:%3 (%4)\n%5")
                        .arg(QString::fromStdString(v.book_name))
                        .arg(v.chapter)
                        .arg(v.verse)
                        .arg(QString::fromStdString(v.translation_abbr))
                        .arg(QString::fromStdString(v.text));
                    QString ui_display = QString("%1  %2").arg(v.verse).arg(QString::fromStdString(v.text));
                    QListWidgetItem* item = new QListWidgetItem(ui_display, results_list);
                    item->setData(Qt::UserRole, payload);
                }
                if (sel_row >= 0 && sel_row < results_list->count()) {
                    results_list->setCurrentRow(sel_row);
                } else if (results_list->count() > 0) {
                    results_list->setCurrentRow(0);
                }
            }
        }
    });
}

std::string HomeInBibleTab::CurrentTranslation() const {
    QString data = translation_combo->currentData().toString();
    return data.isEmpty() ? "KJV" : data.toStdString();
}

void HomeInBibleTab::PopulateTranslations() {
    translation_combo->clear();

    auto translations = bible_db.GetTranslations();
    int default_kjv_idx = 0;
    int idx = 0;

    for (const auto& t : translations) {
        QString abbr = QString::fromStdString(t.abbreviation);
        QString display = abbr;
        if (!t.name.empty()) {
            display = QString("%1 (%2)").arg(abbr, QString::fromStdString(t.name));
        }

        translation_combo->addItem(display, abbr);

        if (abbr.compare("KJV", Qt::CaseInsensitive) == 0) {
            default_kjv_idx = idx;
        }
        idx++;
    }

    if (translation_combo->count() > 0) {
        translation_combo->setCurrentIndex(default_kjv_idx);
    }
}

void HomeInBibleTab::ClearGrid() {
    QLayoutItem *item;
    while ((item = grid_layout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }
}

void HomeInBibleTab::PopulateBookGrid() {
    ClearGrid();
    grid_scroll->setVisible(true);
    results_list->setVisible(false);
    status_label->setText("Select a Book to browse:");
    status_label->setVisible(true);

    auto books = bible_db.GetAllBooks();
    int row = 0, col = 0;
    const int cols = 5;

    for (const auto& book : books) {
        QPushButton* btn = new QPushButton(QString::fromStdString(book), grid_container);
        QFont f("Segoe UI", 10, QFont::Bold);
        f.setWeight(QFont::ExtraBold);
        btn->setFont(f);
        QPalette p = btn->palette();
        p.setColor(QPalette::ButtonText, Qt::white);
        btn->setPalette(p);
        btn->setStyleSheet(
            "QPushButton, QPushButton:enabled { "
            "  background-color: #353b48; "
            "  color: #ffffff !important; "
            "  border: 1px solid #5a657c; "
            "  border-radius: 4px; "
            "  padding: 5px; "
            "  font-size: 12px; "
            "  font-weight: 800; "
            "} "
            "QPushButton:hover { "
            "  background-color: #4b5366; "
            "  color: #ffffff !important; "
            "  border-color: #5294e2; "
            "} "
            "QPushButton:pressed { "
            "  background-color: #5294e2; "
            "  color: #ffffff !important; "
            "}"
        );
        connect(btn, &QPushButton::clicked, this, &HomeInBibleTab::OnBookClicked);
        grid_layout->addWidget(btn, row, col);
        col++;
        if (col >= cols) {
            col = 0;
            row++;
        }
    }
}

void HomeInBibleTab::PopulateChapterGrid(const std::string& book, int chapters) {
    ClearGrid();
    grid_scroll->setVisible(true);
    results_list->setVisible(false);
    current_book = book;
    status_label->setText(QString("%1: Select Chapter (1-%2)").arg(QString::fromStdString(book)).arg(chapters));
    status_label->setVisible(true);

    // Back to Books button
    QPushButton* back_btn = new QPushButton("⬅ Back to Books", grid_container);
    QFont back_f("Segoe UI", 11, QFont::Bold);
    back_f.setWeight(QFont::ExtraBold);
    back_btn->setFont(back_f);
    back_btn->setStyleSheet(
        "QPushButton, QPushButton:enabled { "
        "  background-color: #3b4252; "
        "  color: #88c0d0 !important; "
        "  border: 1px solid #5e6b82; "
        "  border-radius: 4px; "
        "  font-weight: 800; "
        "  padding: 6px; "
        "  font-size: 13px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #434c5e; "
        "  color: #ffffff !important; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #5294e2; "
        "  color: #ffffff !important; "
        "}"
    );
    connect(back_btn, &QPushButton::clicked, this, &HomeInBibleTab::PopulateBookGrid);
    grid_layout->addWidget(back_btn, 0, 0, 1, 4);

    int row = 1, col = 0;
    const int cols = 8;
    for (int i = 1; i <= chapters; ++i) {
        QPushButton* btn = new QPushButton(QString::number(i), grid_container);
        btn->setMinimumSize(38, 34);
        QFont f("Segoe UI", 12, QFont::Bold);
        f.setWeight(QFont::Black);
        btn->setFont(f);
        QPalette p = btn->palette();
        p.setColor(QPalette::ButtonText, Qt::white);
        btn->setPalette(p);
        btn->setStyleSheet(
            "QPushButton, QPushButton:enabled { "
            "  background-color: #353b48; "
            "  color: #ffffff !important; "
            "  border: 1px solid #5a657c; "
            "  border-radius: 4px; "
            "  font-weight: 900; "
            "  font-size: 14px; "
            "} "
            "QPushButton:hover { "
            "  background-color: #4b5366; "
            "  color: #ffffff !important; "
            "  border-color: #7aa2f7; "
            "} "
            "QPushButton:pressed { "
            "  background-color: #3b71ca; "
            "  color: #ffffff !important; "
            "}"
        );
        connect(btn, &QPushButton::clicked, this, &HomeInBibleTab::OnChapterClicked);
        grid_layout->addWidget(btn, row, col);
        col++;
        if (col >= cols) {
            col = 0;
            row++;
        }
    }
}

void HomeInBibleTab::OnBookClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    std::string book = btn->text().toStdString();
    int count = bible_db.GetChapterCount(book);
    if (count <= 0) count = 1;
    PopulateChapterGrid(book, count);
}

void HomeInBibleTab::OnChapterClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int chapter = btn->text().toInt();
    current_chapter = chapter;

    grid_scroll->setVisible(false);
    results_list->setVisible(true);
    results_list->clear();

    auto verses = bible_db.GetChapterVerses(current_book, current_chapter, CurrentTranslation());
    for (const auto& v : verses) {
        QString payload = QString("%1 %2:%3 (%4)\n%5")
            .arg(QString::fromStdString(v.book_name))
            .arg(v.chapter)
            .arg(v.verse)
            .arg(QString::fromStdString(v.translation_abbr))
            .arg(QString::fromStdString(v.text));
        QString ui_display = QString("%1  %2").arg(v.verse).arg(QString::fromStdString(v.text));
        QListWidgetItem* item = new QListWidgetItem(ui_display, results_list);
        item->setData(Qt::UserRole, payload);
    }

    status_label->setText(QString("%1 %2 (%3)").arg(QString::fromStdString(current_book)).arg(current_chapter).arg(QString::fromStdString(CurrentTranslation())));
    status_label->setVisible(true);

    if (results_list->count() > 0) {
        results_list->setCurrentRow(0);
    }
}

void HomeInBibleTab::OnSearchTriggered() {
    QString query = search_box->text().trimmed();
    if (query.isEmpty()) {
        // Clicking search icon with empty query opens the Book Grid
        PopulateBookGrid();
        return;
    }

    // Check if query is just a book name (e.g. "Genesis", "Gen", "1 Cor")
    std::string alias = ResolveBookAlias(query.toStdString());
    std::string check_book = alias.empty() ? query.toStdString() : alias;
    int book_chap_count = bible_db.GetChapterCount(check_book);
    if (book_chap_count > 0) {
        PopulateChapterGrid(check_book, book_chap_count);
        return;
    }

    // Hide grid and show list for verses
    grid_scroll->setVisible(false);
    results_list->setVisible(true);
    results_list->clear();

    std::string trans_abbr = CurrentTranslation();

    // 1. Direct reference with verse or verse range (e.g. "John 3:16" or "1 Cor 13:4-8")
    static QRegularExpression ref_range_regex("^([1-3]?\\s*[A-Za-z]+)\\s+(\\d+)[:\\s]+(\\d+)(?:[-–](\\d+))?$");
    QRegularExpressionMatch match_range = ref_range_regex.match(query);
    if (match_range.hasMatch()) {
        QString raw_book = match_range.captured(1).trimmed();
        std::string resolved = ResolveBookAlias(raw_book.toStdString());
        std::string book = resolved.empty() ? raw_book.toStdString() : resolved;
        int chapter = match_range.captured(2).toInt();
        int v_start = match_range.captured(3).toInt();
        int v_end   = match_range.captured(4).isEmpty() ? v_start : match_range.captured(4).toInt();
        if (v_end < v_start) v_end = v_start;

        current_book = book;
        current_chapter = chapter;

        if (v_start == v_end) {
            BibleVerse pv;
            if (bible_db.GetVerse(book, chapter, v_start, trans_abbr, pv)) {
                QString display = QString("%1 %2:%3 (%4)\n%5")
                    .arg(QString::fromStdString(pv.book_name))
                    .arg(chapter)
                    .arg(v_start)
                    .arg(QString::fromStdString(pv.translation_abbr))
                    .arg(QString::fromStdString(pv.text));

                QListWidgetItem* item = new QListWidgetItem(display, results_list);
                item->setData(Qt::UserRole, display);
                results_list->setCurrentRow(0);
                status_label->setText(QString("%1 %2:%3").arg(QString::fromStdString(pv.book_name)).arg(chapter).arg(v_start));
                status_label->setVisible(true);
                return;
            }
        } else {
            auto range_verses = bible_db.GetRangeVerses(book, chapter, v_start, v_end, trans_abbr);
            if (!range_verses.empty()) {
                QString combined_text;
                for (const auto& rv : range_verses) {
                    if (!combined_text.isEmpty()) combined_text += " ";
                    combined_text += QString("%1 %2").arg(rv.verse).arg(QString::fromStdString(rv.text));
                }
                QString display = QString("%1 %2:%3-%4 (%5)\n%6")
                    .arg(QString::fromStdString(range_verses.front().book_name))
                    .arg(chapter).arg(v_start).arg(v_end).arg(QString::fromStdString(trans_abbr)).arg(combined_text);
                QListWidgetItem* item = new QListWidgetItem(display, results_list);
                item->setData(Qt::UserRole, display);
                results_list->setCurrentRow(0);
                status_label->setText(QString("%1 %2:%3-%4").arg(QString::fromStdString(book)).arg(chapter).arg(v_start).arg(v_end));
                status_label->setVisible(true);
                return;
            }
        }
    }

    // 2. Chapter browsing (e.g. "John 3" or "Psalm 23")
    static QRegularExpression chapter_regex("^([1-3]?\\s*[A-Za-z]+)\\s+(\\d+)$");
    QRegularExpressionMatch match_chap = chapter_regex.match(query);
    if (match_chap.hasMatch()) {
        QString raw_book = match_chap.captured(1).trimmed();
        std::string resolved = ResolveBookAlias(raw_book.toStdString());
        std::string book = resolved.empty() ? raw_book.toStdString() : resolved;
        int chapter = match_chap.captured(2).toInt();

        current_book = book;
        current_chapter = chapter;

        auto chapter_verses = bible_db.GetChapterVerses(book, chapter, trans_abbr);
        if (!chapter_verses.empty()) {
            for (const auto& v : chapter_verses) {
                QString payload = QString("%1 %2:%3 (%4)\n%5")
                    .arg(QString::fromStdString(v.book_name))
                    .arg(v.chapter)
                    .arg(v.verse)
                    .arg(QString::fromStdString(v.translation_abbr))
                    .arg(QString::fromStdString(v.text));
                QString ui_display = QString("%1  %2").arg(v.verse).arg(QString::fromStdString(v.text));
                QListWidgetItem* item = new QListWidgetItem(ui_display, results_list);
                item->setData(Qt::UserRole, payload);
            }
            results_list->setCurrentRow(0);
            status_label->setText(QString("%1 %2 (%3)").arg(QString::fromStdString(book)).arg(chapter).arg(QString::fromStdString(trans_abbr)));
            status_label->setVisible(true);
            return;
        }
    }

    // 3. Fallback to Full-Text Search across verse contents
    auto verses = bible_db.SearchVerses(query.toStdString(), 15);
    for (const auto& v : verses) {
        QString payload = QString("%1 %2:%3 (%4)\n%5")
            .arg(QString::fromStdString(v.book_name))
            .arg(v.chapter)
            .arg(v.verse)
            .arg(QString::fromStdString(v.translation_abbr))
            .arg(QString::fromStdString(v.text));

        QListWidgetItem* item = new QListWidgetItem(payload, results_list);
        item->setData(Qt::UserRole, payload);
    }

    if (results_list->count() > 0) {
        results_list->setCurrentRow(0);
        status_label->setText(QString("Results for: '%1'").arg(query));
        status_label->setVisible(true);
    } else {
        QListWidgetItem* empty_item = new QListWidgetItem(QString("No matching scriptures found for '%1'.").arg(query), results_list);
        empty_item->setFlags(Qt::NoItemFlags);
        status_label->setText("No results");
        status_label->setVisible(true);
    }
}

void HomeInBibleTab::OnPrevVerseClicked() {
    int row = results_list->currentRow();
    if (row > 0) {
        results_list->setCurrentRow(row - 1);
        if (QListWidgetItem* item = results_list->currentItem()) {
            emit RequestPushToOverlay(item->data(Qt::UserRole).toString());
        }
    }
}

void HomeInBibleTab::OnNextVerseClicked() {
    int row = results_list->currentRow();
    if (row < results_list->count() - 1) {
        results_list->setCurrentRow(row + 1);
        if (QListWidgetItem* item = results_list->currentItem()) {
            emit RequestPushToOverlay(item->data(Qt::UserRole).toString());
        }
    }
}

void HomeInBibleTab::OnPushLiveClicked() {
    if (is_live) {
        emit RequestPushToOverlay("");
        return;
    }
    if (QListWidgetItem* item = results_list->currentItem()) {
        emit RequestPushToOverlay(item->data(Qt::UserRole).toString());
    }
}

void HomeInBibleTab::SetLiveState(bool live) {
    is_live = live;
    if (is_live) {
        push_live_btn->setText("Clear Live");
        push_live_btn->setStyleSheet(
            "QPushButton { background-color: #d93025; color: #ffffff; font-weight: bold; border-radius: 4px; padding: 4px; }"
            "QPushButton:hover { background-color: #ea4335; }"
            "QPushButton:pressed { background-color: #b31412; }"
        );
    } else {
        push_live_btn->setText("Push Live");
        push_live_btn->setStyleSheet(
            "QPushButton { background-color: #0d6efd; color: #ffffff; font-weight: bold; border-radius: 4px; padding: 4px; }"
            "QPushButton:hover { background-color: #0b5ed7; }"
            "QPushButton:pressed { background-color: #0a58ca; }"
        );
    }
}

void HomeInBibleTab::OnQueueClicked() {
    if (QListWidgetItem* item = results_list->currentItem()) {
        QString text = item->data(Qt::UserRole).toString();
        emit RequestAddToQueue(text, text, false);
    }
}

void HomeInBibleTab::AutoSearchVerse(const QString& book, int chapter, int verse) {
    search_box->setText(QString("%1 %2:%3").arg(book).arg(chapter).arg(verse));
    OnSearchTriggered();
}

void HomeInBibleTab::PushFirstResultLive() {
    if (results_list->count() <= 0) return;
    results_list->setCurrentRow(0);
    if (QListWidgetItem* item = results_list->currentItem()) {
        emit RequestPushToOverlay(item->data(Qt::UserRole).toString());
    }
}

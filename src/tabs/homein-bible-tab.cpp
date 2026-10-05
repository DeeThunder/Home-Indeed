#include "homein-bible-tab.hpp"
#include <QMessageBox>
#include <QRegularExpression>

HomeInBibleTab::HomeInBibleTab(HomeInDB& db, QWidget *parent) : QWidget(parent), bible_db(db) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Translation Selection Area
    QHBoxLayout* trans_layout = new QHBoxLayout();
    QLabel* prim_label = new QLabel("Primary:");
    primary_trans_combo = new QComboBox();
    QLabel* sec_label = new QLabel("Dual/Secondary:");
    secondary_trans_combo = new QComboBox();

    trans_layout->addWidget(prim_label);
    trans_layout->addWidget(primary_trans_combo, 1);
    trans_layout->addWidget(sec_label);
    trans_layout->addWidget(secondary_trans_combo, 1);
    layout->addLayout(trans_layout);

    // Search Bar Area
    QHBoxLayout* search_layout = new QHBoxLayout();
    search_box = new QLineEdit();
    search_box->setPlaceholderText("e.g. John 3:16 or 'God so loved the world'");
    QPushButton* search_btn = new QPushButton("Search");
    
    search_layout->addWidget(search_box, 1);
    search_layout->addWidget(search_btn);
    layout->addLayout(search_layout);

    // Results List
    results_list = new QListWidget();
    results_list->setWordWrap(true);
    layout->addWidget(results_list);

    // Action Buttons
    QHBoxLayout* action_layout = new QHBoxLayout();
    push_live_btn = new QPushButton("Push Live");
    add_queue_btn = new QPushButton("Add to Queue");
    
    action_layout->addWidget(push_live_btn);
    action_layout->addWidget(add_queue_btn);
    layout->addLayout(action_layout);

    // Populate translations from database
    PopulateTranslations();

    // Connections
    connect(search_btn, &QPushButton::clicked, this, &HomeInBibleTab::OnSearchTriggered);
    connect(search_box, &QLineEdit::returnPressed, this, &HomeInBibleTab::OnSearchTriggered);
    connect(push_live_btn, &QPushButton::clicked, this, &HomeInBibleTab::OnPushLiveClicked);
    connect(add_queue_btn, &QPushButton::clicked, this, &HomeInBibleTab::OnQueueClicked);
}

void HomeInBibleTab::PopulateTranslations() {
    primary_trans_combo->clear();
    secondary_trans_combo->clear();

    secondary_trans_combo->addItem("None (Single Translation)", "");

    auto translations = bible_db.GetTranslations();
    int default_kjv_idx = 0;
    int idx = 0;

    for (const auto& t : translations) {
        QString abbr = QString::fromStdString(t.abbreviation);
        QString display = abbr;
        if (!t.name.empty()) {
            display = QString("%1 (%2)").arg(abbr, QString::fromStdString(t.name));
        }

        primary_trans_combo->addItem(display, abbr);
        secondary_trans_combo->addItem(display, abbr);

        if (abbr.compare("KJV", Qt::CaseInsensitive) == 0) {
            default_kjv_idx = idx;
        }
        idx++;
    }

    if (primary_trans_combo->count() > 0) {
        primary_trans_combo->setCurrentIndex(default_kjv_idx);
    }
    secondary_trans_combo->setCurrentIndex(0); // None by default
}

void HomeInBibleTab::OnSearchTriggered() {
    QString query = search_box->text().trimmed();
    if (query.isEmpty()) return;

    results_list->clear();

    QString prim_abbr = primary_trans_combo->currentData().toString();
    if (prim_abbr.isEmpty()) prim_abbr = "KJV";
    QString sec_abbr = secondary_trans_combo->currentData().toString();

    // 1. Check if query is a direct reference like "John 3:16" or "1 Cor 13:4"
    static QRegularExpression ref_regex("^([1-3]?\\s*[A-Za-z]+)\\s+(\\d+)[:\\s]+(\\d+)$");
    QRegularExpressionMatch match = ref_regex.match(query);

    if (match.hasMatch()) {
        QString book = match.captured(1).trimmed();
        int chapter = match.captured(2).toInt();
        int verse = match.captured(3).toInt();

        BibleVerse pv;
        if (bible_db.GetVerse(book.toStdString(), chapter, verse, prim_abbr.toStdString(), pv)) {
            QString display;
            QString payload;

            if (!sec_abbr.isEmpty() && sec_abbr != prim_abbr) {
                BibleVerse sv;
                if (bible_db.GetVerse(book.toStdString(), chapter, verse, sec_abbr.toStdString(), sv)) {
                    display = QString("%1 %2:%3 (%4 / %5)\n[%4] %6\n[%5] %7")
                        .arg(QString::fromStdString(pv.book_name))
                        .arg(chapter)
                        .arg(verse)
                        .arg(prim_abbr)
                        .arg(sec_abbr)
                        .arg(QString::fromStdString(pv.text))
                        .arg(QString::fromStdString(sv.text));

                    payload = QString("%1 %2:%3 (%4 / %5)\n[%4] %6\n\n[%5] %7")
                        .arg(QString::fromStdString(pv.book_name))
                        .arg(chapter)
                        .arg(verse)
                        .arg(prim_abbr)
                        .arg(sec_abbr)
                        .arg(QString::fromStdString(pv.text))
                        .arg(QString::fromStdString(sv.text));
                } else {
                    display = QString("%1 %2:%3 (%4)\n%5")
                        .arg(QString::fromStdString(pv.book_name))
                        .arg(chapter)
                        .arg(verse)
                        .arg(prim_abbr)
                        .arg(QString::fromStdString(pv.text));
                    payload = display;
                }
            } else {
                display = QString("%1 %2:%3 (%4)\n%5")
                    .arg(QString::fromStdString(pv.book_name))
                    .arg(chapter)
                    .arg(verse)
                    .arg(prim_abbr)
                    .arg(QString::fromStdString(pv.text));
                payload = display;
            }

            QListWidgetItem* item = new QListWidgetItem(display, results_list);
            item->setData(Qt::UserRole, payload);
            return;
        }
    }

    // 2. Fallback to Full-Text Search across verses
    auto verses = bible_db.SearchVerses(query.toStdString(), 10);
    for (const auto& v : verses) {
        QString display;
        QString payload;

        if (!sec_abbr.isEmpty() && sec_abbr.toStdString() != v.translation_abbr) {
            BibleVerse sv;
            if (bible_db.GetVerse(v.book_name, v.chapter, v.verse, sec_abbr.toStdString(), sv)) {
                display = QString("%1 %2:%3 (%4 / %5)\n[%4] %6\n[%5] %7")
                    .arg(QString::fromStdString(v.book_name))
                    .arg(v.chapter)
                    .arg(v.verse)
                    .arg(QString::fromStdString(v.translation_abbr))
                    .arg(sec_abbr)
                    .arg(QString::fromStdString(v.text))
                    .arg(QString::fromStdString(sv.text));

                payload = QString("%1 %2:%3 (%4 / %5)\n[%4] %6\n\n[%5] %7")
                    .arg(QString::fromStdString(v.book_name))
                    .arg(v.chapter)
                    .arg(v.verse)
                    .arg(QString::fromStdString(v.translation_abbr))
                    .arg(sec_abbr)
                    .arg(QString::fromStdString(v.text))
                    .arg(QString::fromStdString(sv.text));
            } else {
                display = QString("%1 %2:%3 (%4)\n%5")
                    .arg(QString::fromStdString(v.book_name))
                    .arg(v.chapter)
                    .arg(v.verse)
                    .arg(QString::fromStdString(v.translation_abbr))
                    .arg(QString::fromStdString(v.text));
                payload = display;
            }
        } else {
            display = QString("%1 %2:%3 (%4)\n%5")
                .arg(QString::fromStdString(v.book_name))
                .arg(v.chapter)
                .arg(v.verse)
                .arg(QString::fromStdString(v.translation_abbr))
                .arg(QString::fromStdString(v.text));
            payload = display;
        }

        QListWidgetItem* item = new QListWidgetItem(display, results_list);
        item->setData(Qt::UserRole, payload);
    }
}

void HomeInBibleTab::OnPushLiveClicked() {
    if (QListWidgetItem* item = results_list->currentItem()) {
        emit RequestPushToOverlay(item->data(Qt::UserRole).toString());
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
    OnPushLiveClicked();
}

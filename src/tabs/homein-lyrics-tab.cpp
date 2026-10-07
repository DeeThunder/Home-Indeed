#include "homein-lyrics-tab.hpp"
#include <QMessageBox>
#include <QSplitter>
#include <QDialog>
#include <QFormLayout>
#include <QTextEdit>

HomeInLyricsTab::HomeInLyricsTab(HomeInLyricsEngine& engine, QWidget *parent) 
    : QWidget(parent), lyrics_engine(engine) {
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);

    // Search Bar
    QHBoxLayout* search_layout = new QHBoxLayout();
    search_box = new QLineEdit();
    search_box->setPlaceholderText("Search song or artist...");
    QPushButton* search_btn = new QPushButton("Search");
    library_btn = new QPushButton("Library");
    add_song_btn = new QPushButton("+ Add Song");
    allow_web_checkbox = new QCheckBox("Web");
    allow_web_checkbox->setToolTip("Allow searching online web lyrics (LRCLIB)");
    allow_web_checkbox->setChecked(true);
    
    search_layout->addWidget(search_box, 1);
    search_layout->addWidget(search_btn);
    search_layout->addWidget(library_btn);
    search_layout->addWidget(add_song_btn);
    search_layout->addWidget(allow_web_checkbox);
    layout->addLayout(search_layout);

    // Splitter for Songs and Sections
    QSplitter* splitter = new QSplitter(Qt::Vertical);
    
    songs_list = new QListWidget();
    songs_list->setMinimumHeight(60);
    sections_list = new QListWidget();
    sections_list->setMinimumHeight(60);
    sections_list->setWordWrap(true);
    
    splitter->addWidget(songs_list);
    splitter->addWidget(sections_list);
    splitter->setChildrenCollapsible(false);
    splitter->setSizes({120, 180});
    layout->addWidget(splitter);

    // Action Buttons
    QHBoxLayout* action_layout = new QHBoxLayout();
    push_live_btn = new QPushButton("Push Live");
    add_queue_btn = new QPushButton("Add to Queue");
    
    action_layout->addWidget(push_live_btn);
    action_layout->addWidget(add_queue_btn);
    layout->addLayout(action_layout);

    // Connections
    connect(search_btn, &QPushButton::clicked, this, &HomeInLyricsTab::OnSearchTriggered);
    connect(library_btn, &QPushButton::clicked, this, &HomeInLyricsTab::OnLibraryClicked);
    connect(add_song_btn, &QPushButton::clicked, this, &HomeInLyricsTab::OnAddSongClicked);
    connect(search_box, &QLineEdit::returnPressed, this, &HomeInLyricsTab::OnSearchTriggered);
    connect(songs_list, &QListWidget::itemSelectionChanged, this, &HomeInLyricsTab::OnSongSelected);
    connect(push_live_btn, &QPushButton::clicked, this, &HomeInLyricsTab::OnPushLiveClicked);
    connect(add_queue_btn, &QPushButton::clicked, this, &HomeInLyricsTab::OnQueueClicked);
    connect(sections_list, &QListWidget::itemDoubleClicked, this, &HomeInLyricsTab::OnPushLiveClicked);

    // Automatically populate library so bundled songs appear on launch
    OnLibraryClicked();
}

void HomeInLyricsTab::OnSearchTriggered() {
    QString query = search_box->text().trimmed();
    if (query.isEmpty()) return;

    songs_list->clear();
    sections_list->clear();
    songs_list->addItem("Searching...");
    const int request_id = ++search_request_id;

    lyrics_engine.Search(query.toStdString(), allow_web_checkbox->isChecked(), 
        [this, request_id](const std::vector<SongLyric>& results) {
            if (request_id != search_request_id) return;
            songs_list->clear();
            if (results.empty()) {
                songs_list->addItem("No results found.");
                return;
            }
            for (const auto& song : results) {
                QString display = QString("%1 - %2").arg(QString::fromStdString(song.title), QString::fromStdString(song.artist));
                QListWidgetItem* item = new QListWidgetItem(display, songs_list);
                item->setData(Qt::UserRole, song.id);
                item->setData(Qt::UserRole + 1, QString::fromStdString(song.content));
            }
        });
}

void HomeInLyricsTab::OnLibraryClicked() {
    songs_list->clear();
    sections_list->clear();
    songs_list->addItem("Loading library...");
    const int request_id = ++search_request_id;

    lyrics_engine.GetLocalLibrary([this, request_id](const std::vector<SongLyric>& results) {
        if (request_id != search_request_id) return;
        songs_list->clear();
        if (results.empty()) {
            songs_list->addItem("Library is empty.");
            return;
        }
        for (const auto& song : results) {
            QString display = QString("%1 - %2").arg(QString::fromStdString(song.title), QString::fromStdString(song.artist));
            QListWidgetItem* item = new QListWidgetItem(display, songs_list);
            item->setData(Qt::UserRole, song.id);
            item->setData(Qt::UserRole + 1, QString::fromStdString(song.content));
        }
    });
}

void HomeInLyricsTab::OnSongSelected() {
    sections_list->clear();
    if (QListWidgetItem* item = songs_list->currentItem()) {
        QString full_lyrics = item->data(Qt::UserRole + 1).toString();
        if (full_lyrics.isEmpty()) {
            SongLyric song;
            if (lyrics_engine.GetDB().FindSongById(item->data(Qt::UserRole).toInt(), song)) {
                full_lyrics = QString::fromStdString(song.content);
                item->setData(Qt::UserRole + 1, full_lyrics);
            }
        }
        if (full_lyrics.isEmpty()) return;

        // Split by natural paragraph breaks (double newlines)
        QString normalized = full_lyrics;
        normalized.replace("\r\n", "\n").replace('\r', '\n');
        QStringList paragraphs = normalized.split(QRegularExpression("\\n\\s*\\n+"), Qt::SkipEmptyParts);

        // Fallback: If no paragraph breaks exist, split by single lines
        if (paragraphs.size() <= 1 && normalized.count('\n') > 4) {
            paragraphs = normalized.split('\n', Qt::SkipEmptyParts);
        }

        for (const QString& para : paragraphs) {
            QString trimmed_para = para.trimmed();
            if (trimmed_para.isEmpty()) continue;

            QStringList lines = trimmed_para.split('\n', Qt::SkipEmptyParts);
            if (lines.size() <= 4) {
                // Natural stanza fits neatly in a single slide
                QListWidgetItem* sec_item = new QListWidgetItem(trimmed_para, sections_list);
                sec_item->setData(Qt::UserRole, "\x01" + trimmed_para);
            } else {
                // Split unusually long stanzas cleanly into 2-to-4 line presentation chunks
                QString current_chunk = "";
                for (int i = 0; i < lines.size(); ++i) {
                    current_chunk += lines[i].trimmed() + "\n";
                    if ((i + 1) % 4 == 0 || i == lines.size() - 1) {
                        QString clean_chunk = current_chunk.trimmed();
                        QListWidgetItem* sec_item = new QListWidgetItem(clean_chunk, sections_list);
                        sec_item->setData(Qt::UserRole, "\x01" + clean_chunk);
                        current_chunk = "";
                    }
                }
            }
        }

        if (sections_list->count() > 0) {
            sections_list->setCurrentRow(0);
        }
    }
}

void HomeInLyricsTab::DisplayLiveMatch(const SongLyric& song, const std::string& spoken_text) {
    // 1. Ensure song is selected in songs_list
    bool found = false;
    for (int i = 0; i < songs_list->count(); ++i) {
        if (songs_list->item(i)->data(Qt::UserRole).toInt() == song.id) {
            songs_list->setCurrentRow(i);
            found = true;
            break;
        }
    }
    if (!found) {
        QString display = QString("%1 - %2").arg(QString::fromStdString(song.title), QString::fromStdString(song.artist));
        QListWidgetItem* item = new QListWidgetItem(display, songs_list);
        item->setData(Qt::UserRole, song.id);
        item->setData(Qt::UserRole + 1, QString::fromStdString(song.content));
        songs_list->setCurrentItem(item);
    }

    // 2. Select matching stanza if spoken_text is provided
    if (sections_list->count() == 0 || spoken_text.empty()) return;

    int best_index = 0;
    int best_score = 0;
    QString q_spoken = QString::fromStdString(spoken_text).toLower();
    QStringList words = q_spoken.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);

    for (int i = 0; i < sections_list->count(); ++i) {
        QString stanza = sections_list->item(i)->text().toLower();
        int score = 0;
        for (const QString& w : words) {
            if (w.length() >= 3 && stanza.contains(w)) {
                score++;
            }
        }
        if (score > best_score) {
            best_score = score;
            best_index = i;
        }
    }

    sections_list->setCurrentRow(best_index);
}

void HomeInLyricsTab::PushFirstResultLive() {
    if (QListWidgetItem* item = sections_list->currentItem()) {
        is_live = true;
        emit RequestPushToOverlay(item->data(Qt::UserRole).toString());
    }
}

void HomeInLyricsTab::OnPushLiveClicked() {
    if (QListWidgetItem* item = sections_list->currentItem()) {
        is_live = true;
        emit RequestPushToOverlay(item->data(Qt::UserRole).toString());
    }
}

void HomeInLyricsTab::AdvanceSlideOnSpeech(const std::string& spoken_text) {
    if (sections_list->count() <= 1 || spoken_text.empty()) return;

    QString q_spoken = QString::fromStdString(spoken_text).toLower();
    QStringList words = q_spoken.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
    if (words.size() < 2) return;

    int current_idx = sections_list->currentRow();
    if (current_idx < 0) current_idx = 0;

    // Check next stanza first for natural forward song progression
    int next_idx = current_idx + 1;
    if (next_idx < sections_list->count()) {
        QString next_stanza = sections_list->item(next_idx)->text().toLower();
        int next_matches = 0;
        for (const QString& w : words) {
            if (w.length() >= 3 && next_stanza.contains(w)) {
                next_matches++;
            }
        }
        if (next_matches >= 2) {
            sections_list->setCurrentRow(next_idx);
            is_live = true;
            emit RequestPushToOverlay(sections_list->item(next_idx)->data(Qt::UserRole).toString());
            return;
        }
    }

    // Check all slides if user jumped (e.g. repeated chorus or bridge)
    int best_idx = -1;
    int best_score = 0;
    for (int i = 0; i < sections_list->count(); ++i) {
        if (i == current_idx) continue;
        QString stanza = sections_list->item(i)->text().toLower();
        int score = 0;
        for (const QString& w : words) {
            if (w.length() >= 3 && stanza.contains(w)) {
                score++;
            }
        }
        if (score > best_score && score >= 3) {
            best_score = score;
            best_idx = i;
        }
    }

    if (best_idx != -1 && best_idx != current_idx) {
        sections_list->setCurrentRow(best_idx);
        is_live = true;
        emit RequestPushToOverlay(sections_list->item(best_idx)->data(Qt::UserRole).toString());
    }
}

void HomeInLyricsTab::OnQueueClicked() {
    if (QListWidgetItem* item = sections_list->currentItem()) {
        QString payload = item->data(Qt::UserRole).toString();
        QString display = payload;
        display.remove("\x01");
        display = display.left(30).replace('\n', ' ') + "...";
        emit RequestAddToQueue(display, payload, true); 
    }
}

void HomeInLyricsTab::OnAddSongClicked() {
    QDialog dlg(this);
    dlg.setWindowTitle("Add / Import Song to Library");
    dlg.setMinimumSize(440, 400);

    QVBoxLayout* dlg_layout = new QVBoxLayout(&dlg);
    dlg_layout->setContentsMargins(12, 12, 12, 12);
    dlg_layout->setSpacing(8);

    QFormLayout* form = new QFormLayout();
    QLineEdit* title_edit = new QLineEdit(&dlg);
    title_edit->setPlaceholderText("e.g. Goodness of God");
    QLineEdit* artist_edit = new QLineEdit(&dlg);
    artist_edit->setPlaceholderText("e.g. Bethel Music / Jenn Johnson");
    form->addRow("Song Title:", title_edit);
    form->addRow("Artist / Author:", artist_edit);
    dlg_layout->addLayout(form);

    // Online Search bar inside dialog
    QHBoxLayout* fetch_layout = new QHBoxLayout();
    QPushButton* fetch_btn = new QPushButton(" Search Online (LRCLIB)", &dlg);
    QLabel* fetch_status = new QLabel("", &dlg);
    fetch_status->setStyleSheet("color: #888; font-size: 11px;");
    fetch_layout->addWidget(fetch_btn);
    fetch_layout->addWidget(fetch_status, 1);
    dlg_layout->addLayout(fetch_layout);

    // Lyrics content text area
    QLabel* content_lbl = new QLabel("Lyrics (paste, type, or auto-fetch):", &dlg);
    QTextEdit* content_edit = new QTextEdit(&dlg);
    content_edit->setPlaceholderText("Paste copied lyrics here, or click 'Search Online' above to automatically populate...");
    dlg_layout->addWidget(content_lbl);
    dlg_layout->addWidget(content_edit, 1);

    // Dialog Buttons
    QHBoxLayout* btn_layout = new QHBoxLayout();
    QPushButton* save_btn = new QPushButton("Save to Library", &dlg);
    QPushButton* cancel_btn = new QPushButton("Cancel", &dlg);
    btn_layout->addStretch();
    btn_layout->addWidget(save_btn);
    btn_layout->addWidget(cancel_btn);
    dlg_layout->addLayout(btn_layout);

    // Fetch from Web logic
    connect(fetch_btn, &QPushButton::clicked, [&]() {
        QString q = title_edit->text().trimmed();
        if (q.isEmpty() && !artist_edit->text().trimmed().isEmpty()) {
            q = artist_edit->text().trimmed();
        } else if (!artist_edit->text().trimmed().isEmpty()) {
            q += " " + artist_edit->text().trimmed();
        }
        if (q.isEmpty()) {
            fetch_status->setText("Enter a title or artist first.");
            return;
        }

        fetch_status->setText("Searching online...");
        fetch_btn->setEnabled(false);

        lyrics_engine.Search(q.toStdString(), true, [&dlg, fetch_btn, fetch_status, title_edit, artist_edit, content_edit](const std::vector<SongLyric>& results) {
            QMetaObject::invokeMethod(&dlg, [&dlg, fetch_btn, fetch_status, title_edit, artist_edit, content_edit, results]() {
                fetch_btn->setEnabled(true);
                if (results.empty()) {
                    fetch_status->setText("No online match found.");
                    return;
                }
                const auto& first = results.front();
                if (title_edit->text().trimmed().isEmpty()) {
                    title_edit->setText(QString::fromStdString(first.title));
                }
                if (artist_edit->text().trimmed().isEmpty()) {
                    artist_edit->setText(QString::fromStdString(first.artist));
                }
                content_edit->setPlainText(QString::fromStdString(first.content));
                fetch_status->setText(QString("Found: %1").arg(QString::fromStdString(first.title)));
            }, Qt::QueuedConnection);
        });
    });

    connect(cancel_btn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(save_btn, &QPushButton::clicked, [&]() {
        QString title = title_edit->text().trimmed();
        QString artist = artist_edit->text().trimmed();
        QString content = content_edit->toPlainText().trimmed();

        if (title.isEmpty()) {
            QMessageBox::warning(&dlg, "Missing Title", "Please enter a song title.");
            return;
        }
        if (content.isEmpty()) {
            QMessageBox::warning(&dlg, "Missing Lyrics", "Please enter or paste the song lyrics.");
            return;
        }

        lyrics_engine.GetDB().AddSong(title.toStdString(), artist.toStdString(), content.toStdString(), "User Import");
        dlg.accept();
    });

    if (dlg.exec() == QDialog::Accepted) {
        OnLibraryClicked();
        QString new_title = title_edit->text().trimmed();
        for (int i = 0; i < songs_list->count(); ++i) {
            if (songs_list->item(i)->text().startsWith(new_title, Qt::CaseInsensitive)) {
                songs_list->setCurrentRow(i);
                break;
            }
        }
    }
}


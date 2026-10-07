#include "homein-dock.hpp"
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include "../audio/homein-audio.hpp"
#include "../renderer/homein-renderer.hpp"
#include "../stt/homein-stt.hpp"
#include "../stt/homein-auto-stt.hpp"
#include <QVBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

// Include your newly isolated tabs
#include "../tabs/homein-transcript-tab.hpp"
#include "../tabs/homein-bible-tab.hpp"
#include "../tabs/homein-lyrics-tab.hpp"
#include "../tabs/homein-queue-tab.hpp"
#include "../tabs/homein-settings-tab.hpp"

#include "homein-downloader.hpp"

static QString FindFirstExistingModel(const QStringList& candidates) {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/HomeIndeed/";
    for (const QString& candidate : candidates) {
        if (QFileInfo::exists(appData + candidate)) return appData + candidate;
        char* module_path = obs_module_file(candidate.toUtf8().constData());
        if (!module_path) continue;
        QString path = QString::fromUtf8(module_path);
        bfree(module_path);
        if (QFileInfo::exists(path)) return path;
    }
    return {};
}


HomeInDock::HomeInDock(QWidget *parent) : QWidget(parent) {
    context = std::make_shared<SermonContext>();
    ref_parser.SetContext(context);
    transcript_queue = std::make_shared<TranscriptQueue>();

    // Load Databases
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/HomeIndeed";
    QDir().mkpath(dataPath);

    // 1. Authoritative Bible DB resolution
    QString biblePath = dataPath + "/homein-bible.db";
    char* mod_file = obs_module_file("homein-bible.db");
    QString installedBiblePath = mod_file ? QString::fromUtf8(mod_file) : QString();
    if (mod_file) bfree(mod_file);

    QString chosenBiblePath = biblePath;
    if (!installedBiblePath.isEmpty() && QFileInfo::exists(installedBiblePath)) {
        // If appData bible is missing or significantly smaller than the installed bundle, sync it
        if (!QFileInfo::exists(biblePath) || QFileInfo(biblePath).size() < QFileInfo(installedBiblePath).size()) {
            QFile::remove(biblePath);
            QFile::copy(installedBiblePath, biblePath);
        }
        chosenBiblePath = QFileInfo::exists(biblePath) ? biblePath : installedBiblePath;
    }
    bible_db.Open(chosenBiblePath.toStdString());

    // 2. Lyrics DB resolution
    QString lyricsPath = dataPath + "/homein-lyrics.db";
    char* mod_lyrics = obs_module_file("homein-lyrics.db");
    QString installedLyricsPath = mod_lyrics ? QString::fromUtf8(mod_lyrics) : QString();
    if (mod_lyrics) bfree(mod_lyrics);

    if (!installedLyricsPath.isEmpty() && QFileInfo::exists(installedLyricsPath)) {
        if (!QFileInfo::exists(lyricsPath) || QFileInfo(lyricsPath).size() < 40000) {
            QFile::remove(lyricsPath);
            QFile::copy(installedLyricsPath, lyricsPath);
        }
    }
    lyrics_engine.Initialize(lyricsPath.toStdString());

    SetupUI();
}

HomeInDock::~HomeInDock() {
    if (render_timer) {
        render_timer->stop();
    }
    StopTranscription();
}

void HomeInDock::SetupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(2, 2, 2, 2);
    main_layout->setSpacing(2);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(280, 220);

    tab_widget = new QTabWidget(this);
    tab_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tab_widget->setDocumentMode(true);

    // 1. Instantiate the Tabs
    transcript_tab = new HomeInTranscriptTab(this);
    bible_tab      = new HomeInBibleTab(bible_db, this);
    lyrics_tab     = new HomeInLyricsTab(lyrics_engine, this);
    queue_tab      = new HomeInQueueTab(this);
    settings_tab   = new HomeInSettingsTab(this);

    // 2. Add them to the QTabWidget
    tab_widget->addTab(transcript_tab, "Transcript");
    tab_widget->addTab(bible_tab, "Bible");
    tab_widget->addTab(lyrics_tab, "Lyrics");
    tab_widget->addTab(queue_tab, "Queue");
    tab_widget->addTab(settings_tab, "Settings");
    main_layout->addWidget(tab_widget);

    // ---------------------------------------------------------
    // 3. THE WIRING: Connecting the isolated tabs together!
    // ---------------------------------------------------------

    // Routing Mic controls to the Dock
    connect(transcript_tab, &HomeInTranscriptTab::RequestToggleMic, this, &HomeInDock::StartTranscription);
    connect(transcript_tab, &HomeInTranscriptTab::RequestPauseMic, this, &HomeInDock::StopTranscription);

    // Routing Bible and Lyrics "Push to Overlay" clicks to the Renderer
    connect(bible_tab, &HomeInBibleTab::RequestPushToOverlay, this, &HomeInDock::PushToOverlay);
    connect(lyrics_tab, &HomeInLyricsTab::RequestPushToOverlay, this, &HomeInDock::PushToOverlay);
    connect(queue_tab, &HomeInQueueTab::RequestPushToOverlay, this, &HomeInDock::PushToOverlay);

    // Routing Bible and Lyrics "Add to Queue" clicks straight to the Queue tab
    connect(bible_tab, &HomeInBibleTab::RequestAddToQueue, queue_tab, &HomeInQueueTab::AddItemToQueue);
    connect(lyrics_tab, &HomeInLyricsTab::RequestAddToQueue, queue_tab, &HomeInQueueTab::AddItemToQueue);

    // Routing Real-time Overlay Styling Updates to Renderer
    connect(settings_tab, &HomeInSettingsTab::OverlaySettingsChanged, this, [this](const OverlaySettings& s) {
        if (HomeInRenderer* renderer = GetActiveRenderer()) {
            renderer->UpdateSettings(s);
            renderer->PrepareTexture();
        }
    });
    if (HomeInRenderer* renderer = GetActiveRenderer()) {
        renderer->UpdateSettings(settings_tab->GetOverlaySettings());
        renderer->PrepareTexture();
    }

    // Dedicated 30ms render & audio monitor loop
    render_timer = new QTimer(this);
    connect(render_timer, &QTimer::timeout, this, [this]() {
        if (HomeInRenderer* renderer = GetActiveRenderer()) {
            renderer->PrepareTexture();
        }
        if (transcript_tab) {
            HomeInAudioHandler* handler = GetAudioHandler();
            if (handler) {
                transcript_tab->UpdateAudioLevel(static_cast<int>(handler->GetLastLevel() * 100.0f));
            }
        }
    });
    render_timer->start(30);
}

void HomeInDock::StartTranscription() {
    if (is_running) return;
    is_running = true;
    
    GetAudioHandler()->Clear();
    transcript_queue->Clear();

    stt_provider = HomeInSTTManager::CreateEngine(STTEngineType::AUTO_LOCAL);

    // --- PREMIUM FEATURE: KEYWORD HARVESTING ---
    // We grab all niche vocabulary from the SQLite databases to bias the AI
    std::vector<std::string> boost_words;

    // 1. Harvest Bible Books
    auto books = bible_db.GetAllBooks();
    boost_words.insert(boost_words.end(), books.begin(), books.end());

    // 2. Harvest Song Titles and Artists
    auto songs = lyrics_engine.GetDB().GetLibrary(5000); // Pull up to 5000 local songs
    for (const auto& song : songs) {
        if (!song.title.empty()) boost_words.push_back(song.title);
        if (!song.artist.empty()) boost_words.push_back(song.artist);
    }

    // 3. Inject into the STT Engine (Only Deepgram will actively use this)
    stt_provider->SetKeywords(boost_words);
    // -------------------------------------------

    transcript_tab->SetAIStatus("Loading AI...", "#ffaa00");

    const QString language = settings_tab->GetLanguageCode();
    const QString performance = settings_tab->GetPerformanceMode();
    ref_parser.SetLanguage(language.toStdString());

    QString preferred_model = FindFirstExistingModel({
        "models/ggml-small.bin",
        "models/ggml-base.bin",
        "models/ggml-small.en.bin",
        "models/ggml-base.en.bin"
    });

    QString fallback_model = FindFirstExistingModel({
        "models/ggml-base.en.bin",
        "models/ggml-base.bin"
    });

    if (preferred_model.isEmpty() && fallback_model.isEmpty()) {
        auto ans = QMessageBox::question(
            this, "AI Speech Model Required",
            "No offline Whisper speech model was found on your system.\n\n"
            "Would you like to download the recommended Whisper model (~140 MB) now?",
            QMessageBox::Yes | QMessageBox::No
        );

        if (ans == QMessageBox::Yes) {
            QString target_file = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/HomeIndeed/models/ggml-base.bin";
            HomeInModelDownloader downloader(target_file, QUrl("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin"), this);
            if (downloader.exec() == QDialog::Accepted && downloader.IsDownloaded()) {
                preferred_model = target_file;
            } else {
                StopTranscription();
                return;
            }
        } else {
            StopTranscription();
            return;
        }
    }

    QtConcurrent::run([this, preferred_model, fallback_model, language, performance]() {
        bool init_success = false;

        AutoSTTConfig config;
        config.model_path = preferred_model.toStdString();
        config.fallback_model_path = fallback_model.toStdString();
        config.language_code = language.toStdString();
        config.performance_mode = performance.toStdString();
        init_success = stt_provider->Initialize(AutoSTTOrchestrator::BuildConfig(config));

        QMetaObject::invokeMethod(this, [this, init_success]() {

            if (!init_success) {
                transcript_tab->SetAIStatus("", "");
                QMessageBox::critical(this, "Speech Engine Error", "Failed to start AI Engine.");
                StopTranscription();
                return;
            }

            transcript_tab->SetAIStatus("● AI Live", "#2aee2a");
            
            stt_provider->Start([this](const std::string& text, bool is_partial) {
                if (!is_partial && !text.empty()) {
                    // Send to queue for the Background Detection Loop
                    transcript_queue->Push(text); 
                    
                    // Also send it straight to the UI
                    QMetaObject::invokeMethod(this, "HandleNewTranscript", Qt::QueuedConnection, Q_ARG(std::string, text));
                }
            });
        }, Qt::QueuedConnection);
    });
}

void HomeInDock::StopTranscription() {
    is_running = false;
    transcript_tab->SetAIStatus("", "");
    if (stt_provider) {
        stt_provider->Stop();
        stt_provider.reset();
    }
}

void HomeInDock::HandleNewTranscript(const std::string& text) {
    // 1. Give text to the UI
    transcript_tab->AppendTranscriptText(QString::fromStdString(text));

    // 1.5 If a song is currently live, check for live slide auto-advancement
    if (lyrics_tab && lyrics_tab->IsLive()) {
        lyrics_tab->AdvanceSlideOnSpeech(text);
    }

    // 2. Send text to the Bible Parser
    auto verses = ref_parser.Parse(text);
    if (!verses.empty()) {
        const auto& best_match = verses.front();
        if (settings_tab->AutoSearch()) {
            bible_tab->AutoSearchVerse(QString::fromStdString(best_match.book), best_match.chapter, best_match.verse_start);
            if (settings_tab->AutoPush()) {
                bible_tab->PushFirstResultLive();
            }
            if (settings_tab->AutoSwitchTabs()) {
                tab_widget->setCurrentWidget(bible_tab);
            }
        }
    } else if (settings_tab->AutoSearch() && text.size() >= 20) {
        // --- PEWBEAM-STYLE SCRIPTURE QUOTE DETECTION ---
        // If no explicit chapter:verse was spoken, search for implied scripture quotes
        auto quote_matches = bible_db.FuzzySearch(text, 1);
        if (!quote_matches.empty()) {
            const auto& qm = quote_matches.front();
            bible_tab->AutoSearchVerse(QString::fromStdString(qm.book_name), qm.chapter, qm.verse);
            if (settings_tab->AutoPush()) {
                bible_tab->PushFirstResultLive();
            }
            if (settings_tab->AutoSwitchTabs()) {
                tab_widget->setCurrentWidget(bible_tab);
            }
        } else if (text.size() >= 10) {
            // 3. Send text to Lyrics Engine if neither reference nor scripture quote was detected
            lyrics_engine.Search(text, false, [this, text](const std::vector<SongLyric>& songs) {
                if (songs.empty()) return;
                QMetaObject::invokeMethod(this, [this, songs, text]() {
                    lyrics_tab->DisplayLiveMatch(songs.front(), text);
                    if (settings_tab->AutoPush()) {
                        lyrics_tab->PushFirstResultLive();
                    }
                    if (settings_tab->AutoSwitchTabs()) {
                        tab_widget->setCurrentWidget(lyrics_tab);
                    }
                }, Qt::QueuedConnection);
            });
        }
    } else if (settings_tab->AutoSearch() && text.size() >= 10) {
        lyrics_engine.Search(text, false, [this, text](const std::vector<SongLyric>& songs) {
            if (songs.empty()) return;
            QMetaObject::invokeMethod(this, [this, songs, text]() {
                lyrics_tab->DisplayLiveMatch(songs.front(), text);
                if (settings_tab->AutoPush()) {
                    lyrics_tab->PushFirstResultLive();
                }
                if (settings_tab->AutoSwitchTabs()) {
                    tab_widget->setCurrentWidget(lyrics_tab);
                }
            }, Qt::QueuedConnection);
        });
    }
}


void HomeInDock::PushToOverlay(const QString& text) {
    bool has_text = !text.trimmed().isEmpty();
    if (HomeInRenderer* renderer = GetActiveRenderer()) {
        renderer->SetText(text.toStdString());
        renderer->PrepareTexture();
    }
    if (bible_tab)  bible_tab->SetLiveState(has_text);
    if (lyrics_tab) lyrics_tab->SetLiveState(has_text);
}

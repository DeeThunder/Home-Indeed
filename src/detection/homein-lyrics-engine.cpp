#include "homein-lyrics-engine.hpp"
#include <QtConcurrent/QtConcurrent>
#include <QtCore/QtCore>
#include <obs-module.h>
#include "network/homein-http.hpp"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <regex>

static void DeduplicateResults(std::vector<SongLyric>& results);

HomeInLyricsEngine::HomeInLyricsEngine() : QObject(nullptr) {}
HomeInLyricsEngine::~HomeInLyricsEngine() {}

bool HomeInLyricsEngine::Initialize(const std::string& db_path) {
    return local_db.Open(db_path);
}

void HomeInLyricsEngine::Search(const std::string& query, bool allow_web, SearchCallback callback) {
    auto local = local_db.Search(query);
    DeduplicateResults(local);

    if (!allow_web) {
        callback(local);
        return;
    }

    FetchFromWeb(query, [local, callback](const std::vector<SongLyric>& web_results) mutable {
        local.insert(local.end(), web_results.begin(), web_results.end());
        DeduplicateResults(local);
        callback(local);
    });
}

void HomeInLyricsEngine::GetLocalLibrary(SearchCallback callback) {
    auto songs = local_db.GetLibrary(5000);
    callback(songs);
}

// Custom C++ HTML Stripper
static std::string StripHTML(const std::string& html) {
    std::string out = html;
    out = std::regex_replace(out, std::regex("<br\\s*/?>", std::regex_constants::icase), "\n");
    out = std::regex_replace(out, std::regex("<[^>]*>"), "");
    out = std::regex_replace(out, std::regex("&#x27;"), "'");
    out = std::regex_replace(out, std::regex("&quot;"), "\"");
    out = std::regex_replace(out, std::regex("&amp;"), "&");
    return out;
}

// --- NEW: Bulletproof Case-Insensitive Deduplication ---
static std::string CleanString(const std::string& str) {
    std::string out;
    for (char c : str) {
        if (std::isalnum(c)) out += std::tolower(c);
    }
    return out;
}

static void DeduplicateResults(std::vector<SongLyric>& results) {
    std::sort(results.begin(), results.end(), [](const SongLyric& a, const SongLyric& b) {
        std::string ta = CleanString(a.title);
        std::string tb = CleanString(b.title);
        if (ta != tb) return ta < tb;
        return CleanString(a.artist) < CleanString(b.artist);
    });

    auto it = std::unique(results.begin(), results.end(),
        [](const SongLyric& a, const SongLyric& b) {
            return CleanString(a.title) == CleanString(b.title) && 
                   CleanString(a.artist) == CleanString(b.artist);
        });
    results.erase(it, results.end());
}
// --------------------------------------------------------

void HomeInLyricsEngine::FetchFromWeb(const std::string& query, SearchCallback callback) {
    blog(LOG_INFO, "HomeIndeed: Searching LRCLIB for: %s", query.c_str());
    
    QtConcurrent::run([this, query, callback]() {
        std::vector<SongLyric> results;
        std::string safe_query = query;
        std::replace(safe_query.begin(), safe_query.end(), ' ', '+');

        std::string lrclib_url = "https://lrclib.net/api/search?q=" + safe_query;
        std::string lrc_resp = HomeIn::HttpClient::Get(lrclib_url);

        if (!lrc_resp.empty()) {
            QJsonDocument lrc_doc = QJsonDocument::fromJson(QByteArray::fromStdString(lrc_resp));
            if (!lrc_doc.isNull() && lrc_doc.isArray()) {
                QJsonArray arr = lrc_doc.array();
                for (int i = 0; i < arr.size(); ++i) {
                    QJsonObject obj = arr[i].toObject();
                    QString plainLyrics = obj["plainLyrics"].toString();
                    QString syncedLyrics = obj["syncedLyrics"].toString();
                    QString lyrics = !plainLyrics.trimmed().isEmpty() ? plainLyrics : syncedLyrics;
                    if (lyrics.contains('[')) {
                        lyrics.remove(QRegularExpression("\\[\\d+:\\d+(?:\\.\\d+)?\\]"));
                    }
                    lyrics = lyrics.trimmed();
                    if (lyrics.isEmpty()) continue;

                    SongLyric s;
                    s.title = obj["trackName"].toString().toStdString();
                    s.artist = obj["artistName"].toString().toStdString();
                    s.content = lyrics.toStdString();
                    s.source = "LRCLIB";
                    results.push_back(s);

                    if (IsDBOpen()) local_db.AddSong(s.title, s.artist, s.content, s.source);
                }
            }
        }

        DeduplicateResults(results);

        QMetaObject::invokeMethod(this, [callback, results]() {
            callback(results);
        }, Qt::QueuedConnection);
    });
}

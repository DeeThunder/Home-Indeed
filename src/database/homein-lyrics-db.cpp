#include "homein-lyrics-db.hpp"
#ifdef HOMEIN_TESTING
#define LOG_INFO 0
#define LOG_WARNING 1
#define LOG_ERROR 2
#define blog(level, ...) do {} while (0)
#else
#include <obs-module.h>
#endif
#include <algorithm>

#include <cctype>
#include <functional>
#include <sstream>

static std::string NormalizeKeyPart(const std::string& text) {
    std::string out;
    for (unsigned char c : text) {
        if (std::isalnum(c)) out.push_back((char)std::tolower(c));
    }
    return out;
}

static std::string BuildSongKey(const std::string& title, const std::string& artist) {
    return NormalizeKeyPart(title) + "|" + NormalizeKeyPart(artist);
}

static std::string BuildContentHash(const std::string& content) {
    std::hash<std::string> hasher;
    std::ostringstream out;
    out << std::hex << hasher(content.substr(0, std::min<size_t>(content.size(), 512)));
    return out.str();
}

#include <unordered_set>

static const std::unordered_set<std::string> STOPWORDS = {
    "a", "an", "the", "and", "or", "but", "if", "because", "as", "what",
    "which", "this", "that", "these", "those", "then", "just", "so", "than",
    "such", "both", "through", "about", "for", "is", "of", "while", "during",
    "to", "from", "in", "out", "on", "off", "over", "under", "again", "further",
    "then", "once", "here", "there", "when", "where", "why", "how", "all", "any",
    "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor",
    "not", "only", "own", "same", "so", "than", "too", "very", "can", "will",
    "just", "don", "should", "now", "i", "me", "my", "myself", "we", "our",
    "ours", "ourselves", "you", "your", "yours", "yourself", "yourselves", "he",
    "him", "his", "himself", "she", "her", "hers", "herself", "it", "its",
    "itself", "they", "them", "their", "theirs", "themselves", "am", "is", "are",
    "was", "were", "be", "been", "being", "have", "has", "had", "having", "do",
    "does", "did", "doing",
    "ti", "ni", "si", "da", "na", "ba", "ya", "ka", "mu", "yi", "za", "ta", "ga"
};

static std::vector<std::string> ExtractTokens(const std::string& query) {
    std::vector<std::string> tokens;
    std::string current;
    for (unsigned char c : query) {
        if (std::isalnum(c)) {
            current.push_back((char)std::tolower(c));
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

static std::string BuildCleanPhrase(const std::vector<std::string>& tokens) {
    std::string out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i) out += " ";
        out += tokens[i];
    }
    return out;
}

static std::string BuildAndFTSQuery(const std::vector<std::string>& tokens) {
    std::string out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i) out += " AND ";
        out += tokens[i] + "*";
    }
    return out;
}

static std::string BuildOrFTSQuery(const std::vector<std::string>& tokens) {
    std::string out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i) out += " OR ";
        out += tokens[i] + "*";
    }
    return out;
}


HomeInLyricsDB::HomeInLyricsDB() {}

HomeInLyricsDB::~HomeInLyricsDB() {
    Close();
}

bool HomeInLyricsDB::Open(const std::string& db_path) {
    if (db) Close();

    blog(LOG_INFO, "HomeIndeed: Opening Lyrics DB at: %s", db_path.c_str());
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) {
        blog(LOG_ERROR, "HomeIndeed: Failed to open Lyrics database: %s",
             sqlite3_errmsg(db));
        db = nullptr;
        return false;
    }

    // FIX #7: Create tables before ValidateDatabase tries to COUNT(*) them.
    EnsureSchema();
    ValidateDatabase();
    return true;
}

// FIX #7: This was the root cause of ALL lyrics failures.
// The homein-lyrics.db file existed on disk but contained zero tables.
// Every call to AddSong / Search / RebuildFTS returned early or failed silently.
void HomeInLyricsDB::EnsureSchema() {
    if (!db) return;

    const char* sql =
        "CREATE TABLE IF NOT EXISTS lyrics ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  title TEXT NOT NULL, artist TEXT,"
        "  content TEXT NOT NULL, source TEXT,"
        "  normalized_key TEXT, content_hash TEXT,"
        "  last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_lyrics_normalized_key "
        "  ON lyrics(normalized_key, content_hash);"
        "CREATE VIRTUAL TABLE IF NOT EXISTS lyrics_fts USING fts5("
        "  title, artist, content,"
        "  content='lyrics', content_rowid='id');"
        "CREATE TRIGGER IF NOT EXISTS lyrics_ai AFTER INSERT ON lyrics BEGIN"
        "  INSERT INTO lyrics_fts(rowid,title,artist,content)"
        "  VALUES(new.id,new.title,new.artist,new.content); END;"
        "CREATE TRIGGER IF NOT EXISTS lyrics_ad AFTER DELETE ON lyrics BEGIN"
        "  INSERT INTO lyrics_fts(lyrics_fts,rowid,title,artist,content)"
        "  VALUES('delete',old.id,old.title,old.artist,old.content); END;"
        "CREATE TRIGGER IF NOT EXISTS lyrics_au AFTER UPDATE ON lyrics BEGIN"
        "  INSERT INTO lyrics_fts(lyrics_fts,rowid,title,artist,content)"
        "  VALUES('delete',old.id,old.title,old.artist,old.content);"
        "  INSERT INTO lyrics_fts(rowid,title,artist,content)"
        "  VALUES(new.id,new.title,new.artist,new.content); END;";

    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        blog(LOG_ERROR, "HomeIndeed: Failed to create lyrics schema: %s", err);
        sqlite3_free(err);
    } else {
        blog(LOG_INFO, "HomeIndeed: Lyrics schema ready.");
    }

    sqlite3_exec(db, "ALTER TABLE lyrics ADD COLUMN normalized_key TEXT;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "ALTER TABLE lyrics ADD COLUMN content_hash TEXT;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "UPDATE lyrics SET normalized_key = lower(replace(title || '|' || ifnull(artist,''), ' ', '')) WHERE normalized_key IS NULL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "UPDATE lyrics SET content_hash = substr(content, 1, 64) WHERE content_hash IS NULL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE UNIQUE INDEX IF NOT EXISTS idx_lyrics_normalized_key ON lyrics(normalized_key, content_hash);", nullptr, nullptr, nullptr);
}

void HomeInLyricsDB::ValidateDatabase() {
    if (!db) return;

    blog(LOG_INFO, "=== HomeIndeed Lyrics DB Diagnostic ===");

    sqlite3_stmt* stmt;
    int count = 0;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM lyrics", -1, &stmt,
                            nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW)
            count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        blog(LOG_INFO, "HomeIndeed: Lyrics table has %d songs cached.", count);
    } else {
        blog(LOG_ERROR, "HomeIndeed: Lyrics table inaccessible: %s",
             sqlite3_errmsg(db));
    }

    blog(LOG_INFO, "=== End Diagnostic ===");
}

void HomeInLyricsDB::Close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool HomeInLyricsDB::AddSong(const std::string& title, const std::string& artist,
                              const std::string& content, const std::string& source) {
    if (!db) return false;

    // FIX: Check if it already exists to prevent SQLite from cloning duplicates!
    SongLyric existing;
    if (FindSong(title, artist, existing)) {
        return true; 
    }

    // INSERT OR REPLACE triggers DELETE then INSERT, which keeps FTS in sync
    const char* sql =
        "INSERT OR IGNORE INTO lyrics (title, artist, content, source, normalized_key, content_hash) "
        "VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, title.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, artist.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, source.c_str(),  -1, SQLITE_TRANSIENT);
    const std::string normalized_key = BuildSongKey(title, artist);
    const std::string content_hash = BuildContentHash(content);
    sqlite3_bind_text(stmt, 5, normalized_key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, content_hash.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool HomeInLyricsDB::FindSong(const std::string& title, const std::string& artist,
                               SongLyric& out_song) {
    if (!db) return false;

    const char* sql =
        "SELECT id, title, artist, content, source FROM lyrics "
        "WHERE title = ? AND artist = ? LIMIT 1";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, title.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, artist.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_song.id      = sqlite3_column_int(stmt, 0);
        out_song.title   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        out_song.artist  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        out_song.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        out_song.source  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

bool HomeInLyricsDB::FindSongById(int id, SongLyric& out_song) {
    if (!db) return false;

    const char* sql =
        "SELECT id, title, artist, content, source FROM lyrics "
        "WHERE id = ? LIMIT 1";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_song.id      = sqlite3_column_int(stmt, 0);
        out_song.title   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* art  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        out_song.artist  = art ? art : "";
        out_song.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* src  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        out_song.source  = src ? src : "";
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

std::vector<SongLyric> HomeInLyricsDB::Search(const std::string& query, int limit) {
    std::vector<SongLyric> results;
    if (!db) return results;

    auto all_tokens = ExtractTokens(query);
    if (all_tokens.empty()) return results;

    std::vector<std::string> meaningful_tokens;
    for (const auto& t : all_tokens) {
        if (STOPWORDS.find(t) == STOPWORDS.end()) {
            meaningful_tokens.push_back(t);
        }
    }
    const auto& tokens_to_use = meaningful_tokens.empty() ? all_tokens : meaningful_tokens;

    std::unordered_set<int> seen_ids;
    auto execute_fts = [&](const std::string& fts_match_expr, bool use_bm25 = false) {
        const char* fts_sql = use_bm25
            ? "SELECT l.id, l.title, l.artist, l.content, l.source "
              "FROM lyrics_fts "
              "JOIN lyrics l ON lyrics_fts.rowid = l.id "
              "WHERE lyrics_fts MATCH ? "
              "ORDER BY bm25(lyrics_fts) "
              "LIMIT ?"
            : "SELECT l.id, l.title, l.artist, l.content, l.source "
              "FROM lyrics_fts "
              "JOIN lyrics l ON lyrics_fts.rowid = l.id "
              "WHERE lyrics_fts MATCH ? "
              "LIMIT ?";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, fts_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, fts_match_expr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, limit - (int)results.size());

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                if (seen_ids.insert(id).second) {
                    SongLyric s;
                    s.id      = id;
                    s.title   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    const char* art = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    s.artist  = art ? art : "";
                    s.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    const char* src = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                    s.source  = src ? src : "";
                    results.push_back(s);
                }
            }
            sqlite3_finalize(stmt);
        }
    };

    // Tier 1: Exact phrase search for multi-word phrases (highest precision)
    if (all_tokens.size() > 1) {
        std::string phrase = "\"" + BuildCleanPhrase(all_tokens) + "\"";
        execute_fts(phrase);
    }

    // Tier 2: Conjunctive AND search for songs containing all meaningful tokens
    if ((int)results.size() < limit && !tokens_to_use.empty()) {
        std::string and_query = BuildAndFTSQuery(tokens_to_use);
        execute_fts(and_query);
    }

    // Tier 3: Ranked BM25 OR search for partial/fuzzy recall
    if ((int)results.size() < limit && !tokens_to_use.empty()) {
        std::string or_query = BuildOrFTSQuery(tokens_to_use);
        execute_fts(or_query, true);
    }

    // Tier 4: Fallback LIKE search if FTS found nothing (punctuation, special chars)
    if (results.empty()) {
        const char* like_sql =
            "SELECT id, title, artist, content, source FROM lyrics "
            "WHERE title LIKE ? OR content LIKE ? OR artist LIKE ? "
            "LIMIT ?";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, like_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            std::string pat = "%" + query + "%";
            sqlite3_bind_text(stmt, 1, pat.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, pat.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, pat.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 4, limit);

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                if (seen_ids.insert(id).second) {
                    SongLyric s;
                    s.id      = id;
                    s.title   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    const char* art = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    s.artist  = art ? art : "";
                    s.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    const char* src = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                    s.source  = src ? src : "";
                    results.push_back(s);
                }
            }
            sqlite3_finalize(stmt);
        }
    }

    return results;
}

std::vector<SongLyric> HomeInLyricsDB::GetLibrary(int limit) {
    std::vector<SongLyric> results;
    if (!db) return results;

    // OPTIMIZATION: Only fetch metadata for the browse list. 
    // Content is fetched individually in FindSong() or OnSongSelected.
    const char* sql = "SELECT id, title, artist, source FROM lyrics ORDER BY title LIMIT ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, limit);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            SongLyric s;
            s.id      = sqlite3_column_int(stmt, 0);
            s.title   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* art = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            s.artist  = art ? art : "";
            s.source  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            results.push_back(s);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

void HomeInLyricsDB::RebuildFTS() {
    if (!db) return;
    // 'rebuild' re-reads all content from the backing lyrics table automatically
    // No need to list columns — safe and always in sync
    char* err = nullptr;
    sqlite3_exec(db,
        "INSERT INTO lyrics_fts(lyrics_fts) VALUES('rebuild');",
        nullptr, nullptr, &err);
    if (err) {
        blog(LOG_ERROR, "HomeIndeed: FTS rebuild failed: %s", err);
        sqlite3_free(err);
    } else {
        blog(LOG_INFO, "HomeIndeed: FTS index rebuilt successfully");
    }
}

#include <gtest/gtest.h>
#include "database/homein-lyrics-db.hpp"

TEST(LyricsDBTest, EmptyDatabaseSearchIsSafe) {
    HomeInLyricsDB db;
    ASSERT_TRUE(db.Open(":memory:"));
    EXPECT_TRUE(db.Search("nothing here").empty());
}

TEST(LyricsDBTest, SearchHandlesPunctuationAndPartialWords) {
    HomeInLyricsDB db;
    ASSERT_TRUE(db.Open(":memory:"));
    ASSERT_TRUE(db.AddSong("Amazing Grace", "Traditional", "Amazing grace how sweet the sound", "test"));

    auto results = db.Search("amazing grace!");
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results.front().title, "Amazing Grace");
}

TEST(LyricsDBTest, LibraryReturnsSavedSongs) {
    HomeInLyricsDB db;
    ASSERT_TRUE(db.Open(":memory:"));
    ASSERT_TRUE(db.AddSong("Way Maker", "Sinach", "Way maker miracle worker", "test"));

    auto library = db.GetLibrary(50);
    ASSERT_EQ(library.size(), 1);

    SongLyric loaded;
    ASSERT_TRUE(db.FindSongById(library.front().id, loaded));
    EXPECT_EQ(loaded.content, "Way maker miracle worker");
}

TEST(LyricsDBTest, PhraseAndStopwordsDisambiguation) {
    HomeInLyricsDB db;
    ASSERT_TRUE(db.Open(":memory:"));
    ASSERT_TRUE(db.AddSong("Song A", "Artist A", "The Lord is in this place and he is good", "test"));
    ASSERT_TRUE(db.AddSong("Song B", "Artist B", "Amazing grace how sweet the sound", "test"));

    // Searching with common stopwords should prioritize the exact phrase/keywords in Song B
    auto results = db.Search("how sweet the sound");
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results.front().title, "Song B");
}


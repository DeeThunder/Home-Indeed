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

TEST(LyricsDBTest, UpdateAndSongDeletion) {
    HomeInLyricsDB db;
    ASSERT_TRUE(db.Open(":memory:"));
    ASSERT_TRUE(db.AddSong("Original Title", "Artist", "Line one\nLine two", "local"));

    auto library = db.GetLibrary(10);
    ASSERT_EQ(library.size(), 1);
    int song_id = library.front().id;

    ASSERT_TRUE(db.UpdateSong(song_id, "Updated Title", "New Artist", "New stanza line 1\nNew stanza line 2"));

    SongLyric loaded;
    ASSERT_TRUE(db.FindSongById(song_id, loaded));
    EXPECT_EQ(loaded.title, "Updated Title");
    EXPECT_EQ(loaded.artist, "New Artist");
    EXPECT_EQ(loaded.content, "New stanza line 1\nNew stanza line 2");

    // Search FTS should find new content
    auto search_results = db.Search("stanza");
    ASSERT_FALSE(search_results.empty());
    EXPECT_EQ(search_results.front().title, "Updated Title");

    // Delete song
    ASSERT_TRUE(db.DeleteSong(song_id));
    EXPECT_FALSE(db.FindSongById(song_id, loaded));
    EXPECT_TRUE(db.Search("stanza").empty());
}


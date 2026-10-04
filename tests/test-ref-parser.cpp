#include <gtest/gtest.h>
#include "detection/homein-ref-parser.hpp"
#include "detection/homein-context.hpp"

TEST(RefParserTest, ParsesEnglishSpokenReference) {
    HomeInRefParser parser;
    auto refs = parser.Parse("Please open your Bible to Genesis chapter five verse twenty two");
    ASSERT_FALSE(refs.empty());
    EXPECT_EQ(refs.front().book, "Genesis");
    EXPECT_EQ(refs.front().chapter, 5);
    EXPECT_EQ(refs.front().verse_start, 22);
}

TEST(RefParserTest, ParsesHausaBookAlias) {
    HomeInRefParser parser;
    parser.SetLanguage("ha");
    auto refs = parser.Parse("Mu bude Farawa 5 aya 22");
    ASSERT_FALSE(refs.empty());
    EXPECT_EQ(refs.front().book, "Genesis");
    EXPECT_EQ(refs.front().chapter, 5);
    EXPECT_EQ(refs.front().verse_start, 22);
}

TEST(RefParserTest, ParsesContextualVerse) {
    auto context = std::make_shared<SermonContext>();
    context->Update("John", 3);

    HomeInRefParser parser;
    parser.SetContext(context);
    auto refs = parser.Parse("verse sixteen");
    ASSERT_FALSE(refs.empty());
    EXPECT_EQ(refs.front().book, "John");
    EXPECT_EQ(refs.front().chapter, 3);
    EXPECT_EQ(refs.front().verse_start, 16);
}

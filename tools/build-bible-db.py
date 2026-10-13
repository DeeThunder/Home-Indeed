import os
import sqlite3
import xml.etree.ElementTree as ET
import sys

# Configuration
XML_DIR = 'vendor/beblia-xml'
DB_PATH = 'data/homein-bible.db'
TRANSLATIONS_TO_IMPORT = [
    ('EnglishKJBible.xml', 'KJV', 'King James Version', 'en', 'English', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('EnglishASVBible.xml', 'ASV', 'American Standard Version', 'en', 'English', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('EnglishYLTBible.xml', 'YLT', 'Young\'s Literal Translation', 'en', 'English', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('EnglishDarbyBible.xml', 'DBY', 'Darby Translation', 'en', 'English', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('EnglishESVBible.xml', 'ESV', 'English Standard Version', 'en', 'English', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('YorubaBible.xml', 'YOR', 'Yoruba Bible', 'yo', 'Yoruba', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('Yoruba2010Bible.xml', 'YOR2010', 'Yoruba Bible 2010', 'yo', 'Yoruba', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('Yoruba2014Bible.xml', 'YOR2014', 'Yoruba Bible 2014', 'yo', 'Yoruba', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('Hausa2010Bible.xml', 'HAU2010', 'Hausa Bible 2010', 'ha', 'Hausa', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('Hausa2013Bible.xml', 'HAU2013', 'Hausa Bible 2013', 'ha', 'Hausa', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('HausaARBible.xml', 'HAUAR', 'Hausa Arabic Script Bible', 'ha', 'Hausa', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('Igbo1988Bible.xml', 'IGB1988', 'Igbo Bible 1988', 'ig', 'Igbo', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('Igbo2006Bible.xml', 'IGB2006', 'Igbo Bible 2006', 'ig', 'Igbo', 'Bundled Beblia XML; verify redistribution terms before release.'),
    ('Igbo2020Bible.xml', 'IGB2020', 'Igbo Bible 2020', 'ig', 'Igbo', 'Bundled Beblia XML; verify redistribution terms before release.'),
]

def build_db():
    print(f"Opening database at {DB_PATH}...")
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()

    # Create tables
    c.execute('DROP TABLE IF EXISTS translations')
    c.execute('DROP TABLE IF EXISTS books')
    c.execute('DROP TABLE IF EXISTS verses')
    
    c.execute('CREATE TABLE translations (id INTEGER PRIMARY KEY, name TEXT, abbreviation TEXT, language_code TEXT, translation_family TEXT, source_notes TEXT)')
    c.execute('CREATE TABLE books (id INTEGER PRIMARY KEY, name TEXT, abbreviation TEXT)')
    c.execute('CREATE VIRTUAL TABLE verses USING fts5(translation_id UNINDEXED, book_id UNINDEXED, chapter UNINDEXED, verse UNINDEXED, text)')

    book_name_to_id = {}
    next_book_id = 1

    for filename, abbr, full_name, language_code, family, source_notes in TRANSLATIONS_TO_IMPORT:
        file_path = os.path.join(XML_DIR, filename)
        if not os.path.exists(file_path):
            print(f"Warning: {file_path} not found, skipping...")
            continue
        
        print(f"Importing {full_name} ({abbr})...")
        c.execute(
            'INSERT INTO translations (name, abbreviation, language_code, translation_family, source_notes) VALUES (?, ?, ?, ?, ?)',
            (full_name, abbr, language_code, family, source_notes),
        )
        translation_id = c.lastrowid

        tree = ET.parse(file_path)
        root = tree.getroot()

        # Beblia structure: bible -> testament -> book -> chapter -> verse
        for testament in root.findall('testament'):
            for book in testament.findall('book'):
                book_name = book.get('name')
                if not book_name:
                    # Some files might use 'number' and we need a lookup, but Beblia usually has names
                    # If name is missing, we'll try to find it or use a default
                    book_name = f"Book {book.get('number')}"
                
                if book_name not in book_name_to_id:
                    c.execute('INSERT INTO books (name) VALUES (?)', (book_name,))
                    book_name_to_id[book_name] = c.lastrowid
                
                book_id = book_name_to_id[book_name]

                for chapter in book.findall('chapter'):
                    chapter_num = chapter.get('number')
                    for verse in chapter.findall('verse'):
                        verse_num = verse.get('number')
                        verse_text = verse.text
                        if verse_text:
                            c.execute('INSERT INTO verses (translation_id, book_id, chapter, verse, text) VALUES (?, ?, ?, ?, ?)',
                                      (translation_id, book_id, chapter_num, verse_num, verse_text))

        conn.commit()
    
    conn.close()
    print("Database build complete!")

if __name__ == "__main__":
    build_db()

#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include "../database/homein-db.hpp"

class HomeInBibleTab : public QWidget {
    Q_OBJECT

public:
    explicit HomeInBibleTab(HomeInDB& db, QWidget *parent = nullptr);

    void AutoSearchVerse(const QString& book, int chapter, int verse);
    void PushFirstResultLive();

signals:
    void RequestPushToOverlay(const QString& text);
    void RequestAddToQueue(const QString& display_text, const QString& payload, bool is_lyrics);

public slots:
    void PopulateBookGrid();
    void PopulateChapterGrid(const std::string& book, int chapters);

private:
    HomeInDB& bible_db;
    QLineEdit* search_box;
    QPushButton* search_btn;
    QPushButton* browse_books_btn;
    QLabel* status_label;

    QScrollArea* grid_scroll;
    QWidget* grid_container;
    QGridLayout* grid_layout;

    QListWidget* results_list;
    QPushButton* prev_verse_btn;
    QPushButton* next_verse_btn;
    QPushButton* push_live_btn;
    QPushButton* add_queue_btn;
    QComboBox* translation_combo;

    std::string current_book;
    int current_chapter = 0;

    void PopulateTranslations();
    void ClearGrid();
    std::string CurrentTranslation() const;

private slots:
    void OnSearchTriggered();
    void OnBookClicked();
    void OnChapterClicked();
    void OnPrevVerseClicked();
    void OnNextVerseClicked();
    void OnPushLiveClicked();
    void OnQueueClicked();
};

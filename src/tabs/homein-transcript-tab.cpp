#include "homein-transcript-tab.hpp"

HomeInTranscriptTab::HomeInTranscriptTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);

    // Toolbar
    QHBoxLayout* toolbar = new QHBoxLayout();
    toolbar->setContentsMargins(0, 0, 0, 0);
    toolbar->setSpacing(4);
    mic_btn = new QPushButton(" Start Mic");
    mic_btn->setToolTip("Start live speech transcription");
    pause_btn = new QPushButton(" Pause");
    pause_btn->setEnabled(false);
    clear_btn = new QPushButton(" Clear");
    clear_btn->setToolTip("Clear the transcript history");
    ai_status_label = new QLabel("", this);

    toolbar->addWidget(mic_btn);
    toolbar->addWidget(pause_btn);
    toolbar->addWidget(clear_btn);
    toolbar->addWidget(ai_status_label);
    toolbar->addStretch();
    layout->addLayout(toolbar);

    // Audio Level Bar
    audio_level_bar = new QProgressBar();
    audio_level_bar->setMaximumHeight(4);
    audio_level_bar->setTextVisible(false);
    layout->addWidget(audio_level_bar);

    // Live AI Preview
    live_preview_box = new QLineEdit();
    live_preview_box->setReadOnly(true);
    live_preview_box->setPlaceholderText("Live speech preview...");
    layout->addWidget(live_preview_box);

    // Final Transcript Box
    transcript_box = new QTextEdit();
    transcript_box->setReadOnly(true);
    transcript_box->setPlaceholderText("Spoken sermon transcript will appear here when microphone is active...");
    transcript_box->setMinimumHeight(60);
    layout->addWidget(transcript_box, 1);

    // Wire Buttons to Signals
    connect(mic_btn, &QPushButton::clicked, this, &HomeInTranscriptTab::RequestToggleMic);
    connect(pause_btn, &QPushButton::clicked, this, &HomeInTranscriptTab::RequestPauseMic);
    connect(clear_btn, &QPushButton::clicked, transcript_box, &QTextEdit::clear);
}

void HomeInTranscriptTab::AppendTranscriptText(const QString& text) {
    transcript_box->append(text);
    live_preview_box->clear(); // Clear the partial box when a full sentence finishes
}

void HomeInTranscriptTab::SetAIStatus(const QString& status, const QString& colorHex) {
    if (status.isEmpty()) {
        ai_status_label->setText("");
    } else {
        ai_status_label->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px;").arg(colorHex));
        ai_status_label->setText(status);
    }
}
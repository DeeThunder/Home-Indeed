#pragma once
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QSlider>
#include "../renderer/homein-renderer.hpp"

class HomeInSettingsTab : public QWidget {
    Q_OBJECT
public:
    explicit HomeInSettingsTab(QWidget *parent = nullptr);

    // Getters so the Main Dock can read settings when pushing text
    bool AutoSwitchTabs() const;
    bool AutoSearch() const;
    bool AutoPush() const;
    QString GetDeepgramKey() const;
    QString GetLanguageCode() const;
    QString GetPerformanceMode() const;
    int GetLinesPerPage() const;
    OverlaySettings GetOverlaySettings() const;

signals:
    // Shouts when the audio source changes so the Audio Handler can restart
    void AudioSourceChanged(const QString& source_name);
    // Real-time styling update signal for the renderer
    void OverlaySettingsChanged(const OverlaySettings& settings);

private:
    QComboBox* language_combo;
    QComboBox* performance_combo;
    QCheckBox* auto_switch_tabs_checkbox;
    QCheckBox* auto_search_checkbox;
    QCheckBox* auto_push_checkbox;
    QLineEdit* deepgram_key_edit;

    // Overlay Styling Controls
    QFontComboBox* font_family_combo;
    QSpinBox* font_size_spin;
    QComboBox* font_color_combo;
    QComboBox* align_combo;
    QCheckBox* text_shadow_check;
    QCheckBox* show_bg_check;
    QSlider* bg_opacity_slider;
    QSpinBox* margin_spin;
    QSpinBox* lower_third_spin;

    void LoadSettings();
    void EmitSettingsChanged();

private slots:
    void SaveSettings();
    void OnStylingChanged();
};

#include "homein-settings-tab.hpp"
#include <QSettings>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>

HomeInSettingsTab::HomeInSettingsTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    // 1. AI Engine Group
    QGroupBox* ai_group = new QGroupBox("AI Engine");
    QVBoxLayout* ai_layout = new QVBoxLayout(ai_group);
    language_combo = new QComboBox();
    language_combo->addItem("Auto Detect", "auto");
    language_combo->addItem("English", "en");
    language_combo->addItem("Yoruba", "yo");
    language_combo->addItem("Hausa", "ha");
    language_combo->addItem("Igbo", "ig");

    performance_combo = new QComboBox();
    performance_combo->addItem("Balanced", "balanced");
    performance_combo->addItem("Fast", "fast");
    performance_combo->addItem("Accurate", "accurate");

    deepgram_key_edit = new QLineEdit();
    deepgram_key_edit->setPlaceholderText("Optional cloud STT key for developer fallback");
    deepgram_key_edit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ai_layout->addWidget(language_combo);
    ai_layout->addWidget(performance_combo);
    ai_layout->addWidget(deepgram_key_edit);
    layout->addWidget(ai_group);

    // 2. Automation Group
    QGroupBox* auto_group = new QGroupBox("Automation");
    QVBoxLayout* auto_layout = new QVBoxLayout(auto_group);
    auto_search_checkbox = new QCheckBox("Auto-Search Bible Verses");
    auto_push_checkbox = new QCheckBox("Auto-Push Verses to Live Screen");
    auto_switch_tabs_checkbox = new QCheckBox("Auto-Switch to Bible Tab on Match");
    auto_layout->addWidget(auto_search_checkbox);
    auto_layout->addWidget(auto_push_checkbox);
    auto_layout->addWidget(auto_switch_tabs_checkbox);
    layout->addWidget(auto_group);

    // 3. Overlay Appearance & Styling Editor Group
    QGroupBox* style_group = new QGroupBox("Overlay Appearance & Styling");
    QFormLayout* style_layout = new QFormLayout(style_group);

    font_family_combo = new QFontComboBox();
    style_layout->addRow("Font Family:", font_family_combo);

    font_size_spin = new QSpinBox();
    font_size_spin->setRange(16, 96);
    font_size_spin->setValue(48);
    font_size_spin->setSuffix(" pt");
    style_layout->addRow("Font Size:", font_size_spin);

    font_color_combo = new QComboBox();
    font_color_combo->addItem("White", "white");
    font_color_combo->addItem("Black", "black");
    style_layout->addRow("Font Color:", font_color_combo);

    align_combo = new QComboBox();
    align_combo->addItem("Center", (int)Qt::AlignCenter);
    align_combo->addItem("Left", (int)Qt::AlignLeft);
    align_combo->addItem("Right", (int)Qt::AlignRight);
    style_layout->addRow("Text Alignment:", align_combo);

    text_shadow_check = new QCheckBox("Enable Text Shadow / Outline");
    style_layout->addRow("", text_shadow_check);

    show_bg_check = new QCheckBox("Show Background Bounding Box");
    style_layout->addRow("", show_bg_check);

    bg_opacity_slider = new QSlider(Qt::Horizontal);
    bg_opacity_slider->setRange(0, 255);
    bg_opacity_slider->setValue(180);
    style_layout->addRow("Background Opacity:", bg_opacity_slider);

    margin_spin = new QSpinBox();
    margin_spin->setRange(20, 400);
    margin_spin->setValue(80);
    margin_spin->setSuffix(" px");
    style_layout->addRow("Side Margins:", margin_spin);

    lower_third_spin = new QSpinBox();
    lower_third_spin->setRange(0, 400);
    lower_third_spin->setValue(40);
    lower_third_spin->setSuffix(" px");
    style_layout->addRow("Bottom Offset:", lower_third_spin);

    layout->addWidget(style_group);

    // Save button
    QPushButton* save_btn = new QPushButton("Save Settings");
    layout->addWidget(save_btn);
    layout->addStretch();

    // Wire styling controls for real-time live preview
    connect(font_family_combo, &QFontComboBox::currentFontChanged, this, &HomeInSettingsTab::OnStylingChanged);
    connect(font_size_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &HomeInSettingsTab::OnStylingChanged);
    connect(font_color_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HomeInSettingsTab::OnStylingChanged);
    connect(align_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HomeInSettingsTab::OnStylingChanged);
    connect(text_shadow_check, &QCheckBox::toggled, this, &HomeInSettingsTab::OnStylingChanged);
    connect(show_bg_check, &QCheckBox::toggled, this, &HomeInSettingsTab::OnStylingChanged);
    connect(bg_opacity_slider, &QSlider::valueChanged, this, &HomeInSettingsTab::OnStylingChanged);
    connect(margin_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &HomeInSettingsTab::OnStylingChanged);
    connect(lower_third_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &HomeInSettingsTab::OnStylingChanged);

    connect(save_btn, &QPushButton::clicked, this, &HomeInSettingsTab::SaveSettings);

    LoadSettings();
}

void HomeInSettingsTab::LoadSettings() {
    QSettings settings("HomeIndeed", "Plugin");
    deepgram_key_edit->setText(settings.value("deepgram_key", "").toString());
    const QString language = settings.value("language_code", "auto").toString();
    int lang_index = language_combo->findData(language);
    language_combo->setCurrentIndex(lang_index >= 0 ? lang_index : 0);

    const QString perf = settings.value("performance_mode", "balanced").toString();
    int perf_index = performance_combo->findData(perf);
    performance_combo->setCurrentIndex(perf_index >= 0 ? perf_index : 0);

    auto_search_checkbox->setChecked(settings.value("auto_search", true).toBool());
    auto_push_checkbox->setChecked(settings.value("auto_push", true).toBool());
    auto_switch_tabs_checkbox->setChecked(settings.value("auto_switch", true).toBool());

    // Styling settings
    QString family = settings.value("font_family", "Arial").toString();
    font_family_combo->setCurrentFont(QFont(family));
    font_size_spin->setValue(settings.value("base_font_size", 48).toInt());

    QString color = settings.value("font_color", "white").toString();
    int color_idx = font_color_combo->findData(color);
    font_color_combo->setCurrentIndex(color_idx >= 0 ? color_idx : 0);

    int align_val = settings.value("alignment", (int)Qt::AlignCenter).toInt();
    int align_idx = align_combo->findData(align_val);
    align_combo->setCurrentIndex(align_idx >= 0 ? align_idx : 0);

    text_shadow_check->setChecked(settings.value("text_shadow", true).toBool());
    show_bg_check->setChecked(settings.value("show_background", true).toBool());
    bg_opacity_slider->setValue(settings.value("bg_opacity", 180).toInt());
    margin_spin->setValue(settings.value("margin", 80).toInt());
    lower_third_spin->setValue(settings.value("lower_third_offset", 40).toInt());
}

void HomeInSettingsTab::SaveSettings() {
    QSettings settings("HomeIndeed", "Plugin");
    settings.setValue("deepgram_key", deepgram_key_edit->text());
    settings.setValue("language_code", language_combo->currentData().toString());
    settings.setValue("performance_mode", performance_combo->currentData().toString());
    settings.setValue("auto_search", auto_search_checkbox->isChecked());
    settings.setValue("auto_push", auto_push_checkbox->isChecked());
    settings.setValue("auto_switch", auto_switch_tabs_checkbox->isChecked());

    // Save styling settings
    settings.setValue("font_family", font_family_combo->currentFont().family());
    settings.setValue("base_font_size", font_size_spin->value());
    settings.setValue("font_color", font_color_combo->currentData().toString());
    settings.setValue("alignment", align_combo->currentData().toInt());
    settings.setValue("text_shadow", text_shadow_check->isChecked());
    settings.setValue("show_background", show_bg_check->isChecked());
    settings.setValue("bg_opacity", bg_opacity_slider->value());
    settings.setValue("margin", margin_spin->value());
    settings.setValue("lower_third_offset", lower_third_spin->value());

    EmitSettingsChanged();
}

OverlaySettings HomeInSettingsTab::GetOverlaySettings() const {
    OverlaySettings s;
    s.font_family = font_family_combo->currentFont().family();
    s.base_font_size = font_size_spin->value();
    s.font_color = font_color_combo->currentData().toString();
    s.alignment = (Qt::Alignment)align_combo->currentData().toInt();
    s.text_shadow = text_shadow_check->isChecked();
    s.show_background = show_bg_check->isChecked();
    s.bg_opacity = bg_opacity_slider->value();
    s.margin = margin_spin->value();
    s.lower_third_offset = lower_third_spin->value();
    return s;
}

void HomeInSettingsTab::OnStylingChanged() {
    EmitSettingsChanged();
}

void HomeInSettingsTab::EmitSettingsChanged() {
    emit OverlaySettingsChanged(GetOverlaySettings());
}

bool HomeInSettingsTab::AutoSearch() const { return auto_search_checkbox->isChecked(); }
bool HomeInSettingsTab::AutoPush() const { return auto_push_checkbox->isChecked(); }
bool HomeInSettingsTab::AutoSwitchTabs() const { return auto_switch_tabs_checkbox->isChecked(); }
QString HomeInSettingsTab::GetDeepgramKey() const { return deepgram_key_edit->text(); }
QString HomeInSettingsTab::GetLanguageCode() const { return language_combo->currentData().toString(); }
QString HomeInSettingsTab::GetPerformanceMode() const { return performance_combo->currentData().toString(); }
int HomeInSettingsTab::GetLinesPerPage() const { return 4; }

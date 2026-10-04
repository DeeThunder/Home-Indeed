#include "homein-auto-stt.hpp"
#include <obs-module.h>
#include <sstream>
#include <vector>

AutoSTTOrchestrator::AutoSTTOrchestrator() {}

AutoSTTOrchestrator::~AutoSTTOrchestrator() {
    Stop();
}

std::string AutoSTTOrchestrator::BuildConfig(const AutoSTTConfig& config) {
    return config.model_path + "\n" +
           config.fallback_model_path + "\n" +
           config.language_code + "\n" +
           config.performance_mode;
}

AutoSTTConfig AutoSTTOrchestrator::ParseConfig(const std::string& config) {
    AutoSTTConfig parsed;
    std::stringstream ss(config);
    std::getline(ss, parsed.model_path);
    std::getline(ss, parsed.fallback_model_path);
    std::getline(ss, parsed.language_code);
    std::getline(ss, parsed.performance_mode);
    if (parsed.language_code.empty()) parsed.language_code = "auto";
    if (parsed.performance_mode.empty()) parsed.performance_mode = "balanced";
    return parsed;
}

bool AutoSTTOrchestrator::Initialize(const std::string& config) {
    active_config = ParseConfig(config);
    whisper_engine = std::make_unique<HomeInWhisperEngine>();
    whisper_engine->SetLanguage(active_config.language_code);
    whisper_engine->SetPerformanceMode(active_config.performance_mode);

    if (!active_config.model_path.empty() && whisper_engine->Initialize(active_config.model_path)) {
        blog(LOG_INFO, "HomeIndeed: Local AI loaded primary model: %s", active_config.model_path.c_str());
        return true;
    }

    if (!active_config.fallback_model_path.empty() && whisper_engine->Initialize(active_config.fallback_model_path)) {
        blog(LOG_WARNING, "HomeIndeed: Local AI fell back to bundled model: %s", active_config.fallback_model_path.c_str());
        return true;
    }

    blog(LOG_ERROR, "HomeIndeed: Local AI could not load any Whisper model.");
    whisper_engine.reset();
    return false;
}

void AutoSTTOrchestrator::Start(TranscriptCallback callback) {
    if (whisper_engine) whisper_engine->Start(callback);
}

void AutoSTTOrchestrator::Stop() {
    if (whisper_engine) whisper_engine->Stop();
}

void AutoSTTOrchestrator::SetPaused(bool paused) {
    if (whisper_engine) whisper_engine->SetPaused(paused);
}

bool AutoSTTOrchestrator::IsPaused() const {
    return whisper_engine ? whisper_engine->IsPaused() : false;
}

bool AutoSTTOrchestrator::IsRunning() const {
    return whisper_engine ? whisper_engine->IsRunning() : false;
}

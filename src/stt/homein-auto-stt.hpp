#pragma once

#include "ISTTProvider.hpp"
#include "homein-whisper.hpp"
#include <memory>
#include <string>

struct AutoSTTConfig {
    std::string model_path;
    std::string fallback_model_path;
    std::string language_code = "auto";
    std::string performance_mode = "balanced";
};

class AutoSTTOrchestrator : public ISTTProvider {
public:
    AutoSTTOrchestrator();
    ~AutoSTTOrchestrator() override;

    bool Initialize(const std::string& config) override;
    void Start(TranscriptCallback callback) override;
    void Stop() override;
    void SetPaused(bool paused) override;
    bool IsPaused() const override;
    bool IsRunning() const override;
    std::string GetName() const override { return "Home Indeed Local AI"; }
    bool IsCloud() const override { return false; }

    static std::string BuildConfig(const AutoSTTConfig& config);

private:
    static AutoSTTConfig ParseConfig(const std::string& config);
    std::unique_ptr<HomeInWhisperEngine> whisper_engine;
    AutoSTTConfig active_config;
};

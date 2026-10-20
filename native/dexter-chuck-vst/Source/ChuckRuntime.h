#pragma once

#include "chuck.h"

#include <memory>
#include <mutex>
#include <string>

class ChuckRuntime
{
public:
    ChuckRuntime() = default;
    ~ChuckRuntime() = default;

    ChuckRuntime(const ChuckRuntime&) = delete;
    ChuckRuntime& operator=(const ChuckRuntime&) = delete;

    bool initialise(int sampleRate, int inputChannels, int outputChannels,
                    std::string& error);
    bool compile(const std::string& code, const std::string& sourcePath,
                 std::string& error);
    void stop();
    void setSampleRate(double sampleRate);
    bool process(const float* input, float* output, int frames) noexcept;

private:
    std::unique_ptr<ChucK> createEngine(std::string& error) const;

    static std::mutex compilerMutex;
    mutable std::recursive_mutex runtimeMutex;
    std::unique_ptr<ChucK> engine;
    int sampleRateHz = 44100;
    int inputChannels = 0;
    int outputChannels = 2;
};

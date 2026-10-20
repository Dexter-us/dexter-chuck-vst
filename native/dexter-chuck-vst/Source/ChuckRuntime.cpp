#include "ChuckRuntime.h"

#include "chuck_errmsg.h"

#include <algorithm>
#include <vector>

std::mutex ChuckRuntime::compilerMutex;

bool ChuckRuntime::initialise(int sampleRate, int inputChannels, int outputChannels,
                              std::string& error)
{
    std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
    sampleRateHz = sampleRate;
    this->inputChannels = inputChannels;
    this->outputChannels = outputChannels;
    engine = createEngine(error);
    if (!engine)
        return false;
    error.clear();
    return true;
}

bool ChuckRuntime::compile(const std::string& code, const std::string& sourcePath,
                           std::string& error)
{
    std::lock_guard<std::mutex> compilerLock(compilerMutex);
    std::lock_guard<std::recursive_mutex> runtimeLock(runtimeMutex);
    if (!engine)
    {
        engine = createEngine(error);
        if (!engine)
            return false;
    }

    std::vector<t_CKUINT> shredIds;
    EM_reset_msg();
    if (!engine->compileCode(code, "", 1, FALSE, &shredIds, sourcePath))
    {
        error = EM_lasterror();
        if (error.empty())
            error = "ChucK could not compile this script.";
        return false;
    }

    // Compile into the active VM: ChucK starts a new shred while existing
    // shreds and their UGen graphs continue running until Stop resets the VM.
    error.clear();
    return true;
}

std::unique_ptr<ChucK> ChuckRuntime::createEngine(std::string& error) const
{
    auto candidate = std::make_unique<ChucK>();
    candidate->setParam(CHUCK_PARAM_SAMPLE_RATE, sampleRateHz);
    candidate->setParam(CHUCK_PARAM_INPUT_CHANNELS, inputChannels);
    candidate->setParam(CHUCK_PARAM_OUTPUT_CHANNELS, outputChannels);
    candidate->setParam(CHUCK_PARAM_VM_HALT, FALSE);
    candidate->setParam(CHUCK_PARAM_OTF_ENABLE, FALSE);
    candidate->setParam(CHUCK_PARAM_CHUGIN_ENABLE, FALSE);
    candidate->setParam(CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, TRUE);

    if (!candidate->init() || !candidate->start())
    {
        error = EM_lasterror();
        if (error.empty())
            error = "The ChucK audio engine could not be initialized.";
        return {};
    }
    return candidate;
}

void ChuckRuntime::stop()
{
    std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
    std::string ignoredError;
    engine = createEngine(ignoredError);
}

void ChuckRuntime::setSampleRate(double sampleRate)
{
    std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
    sampleRateHz = static_cast<int>(sampleRate);
    if (engine && engine->vm() != nullptr)
        engine->vm()->update_srate(static_cast<t_CKUINT>(sampleRateHz));
}

bool ChuckRuntime::process(const float* input, float* output, int frames) noexcept
{
    if (frames <= 0 || output == nullptr)
        return false;

    std::unique_lock<std::recursive_mutex> lock(runtimeMutex, std::try_to_lock);
    if (!lock.owns_lock() || !engine)
    {
        std::fill_n(output, frames * 2, 0.0f);
        return false;
    }

    engine->run(input, output, frames);
    return true;
}

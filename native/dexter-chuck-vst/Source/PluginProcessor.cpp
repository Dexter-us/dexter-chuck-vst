#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

#include <array>
#include <cstring>

namespace
{
constexpr int maxChunkFrames = 512;

std::string readEmbeddedExample(const char* resourceName)
{
    int byteCount = 0;
    if (const auto* data = BinaryData::getNamedResource(resourceName, byteCount))
        return std::string(data, static_cast<size_t>(byteCount));
    return {};
}
}

DexterChuckAudioProcessor::DexterChuckAudioProcessor()
    : juce::AudioProcessor(BusesProperties()
#if DEXTER_CHUCK_IS_EFFECT
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    std::string error;
    const int inputChannels = isEffect() ? 2 : 0;
    if (!runtime.initialise(44100, inputChannels, 2, error))
    {
        startupError = error;
        return;
    }

    currentCode = getExampleCode();
    if (!currentCode.empty() && !runtime.compile(currentCode, {}, error))
        startupError = error;
}

void DexterChuckAudioProcessor::prepareToPlay(double sampleRate, int)
{
    runtime.setSampleRate(sampleRate);
}

void DexterChuckAudioProcessor::releaseResources()
{
}

bool DexterChuckAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#if DEXTER_CHUCK_IS_EFFECT
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
#else
    return layouts.getMainInputChannelSet().isDisabled();
#endif
}

void DexterChuckAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    midiMessages.clear();

    if (codeStopped.load(std::memory_order_acquire))
    {
        buffer.clear();
        return;
    }

    if (buffer.getNumChannels() < 2)
    {
        buffer.clear();
        return;
    }

    std::array<float, maxChunkFrames * 2> input {};
    std::array<float, maxChunkFrames * 2> output {};
    const int inputChannels = getTotalNumInputChannels();

    for (int offset = 0; offset < buffer.getNumSamples(); offset += maxChunkFrames)
    {
        const int frames = juce::jmin(maxChunkFrames, buffer.getNumSamples() - offset);
        const float* inputPointer = nullptr;
        if (inputChannels >= 2)
        {
            for (int frame = 0; frame < frames; ++frame)
            {
                input[static_cast<size_t>(frame) * 2] = buffer.getSample(0, offset + frame);
                input[static_cast<size_t>(frame) * 2 + 1] = buffer.getSample(1, offset + frame);
            }
            inputPointer = input.data();
        }

        if (!runtime.process(inputPointer, output.data(), frames))
            std::fill_n(output.data(), frames * 2, 0.0f);

        for (int frame = 0; frame < frames; ++frame)
        {
            buffer.setSample(0, offset + frame, output[static_cast<size_t>(frame) * 2]);
            buffer.setSample(1, offset + frame, output[static_cast<size_t>(frame) * 2 + 1]);
        }
    }
}

juce::AudioProcessorEditor* DexterChuckAudioProcessor::createEditor()
{
    return new DexterChuckAudioProcessorEditor(*this, startupError);
}

const juce::String DexterChuckAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void DexterChuckAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    stream.writeInt(1);
    stream.writeString(juce::String::fromUTF8(getCurrentCode().c_str()));
}

void DexterChuckAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes < static_cast<int>(sizeof(int))
        || sizeInBytes > 1024 * 1024 + 32)
        return;

    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    if (stream.readInt() != 1)
        return;
    const auto restored = stream.readString();
    if (restored.getNumBytesAsUTF8() <= 1024 * 1024)
        setCurrentCode(restored.toStdString());
}

bool DexterChuckAudioProcessor::isEffect() const noexcept
{
#if DEXTER_CHUCK_IS_EFFECT
    return true;
#else
    return false;
#endif
}

std::string DexterChuckAudioProcessor::getCurrentCode() const
{
    std::lock_guard<std::mutex> lock(codeMutex);
    return currentCode;
}

void DexterChuckAudioProcessor::setCurrentCode(const std::string& code)
{
    std::lock_guard<std::mutex> lock(codeMutex);
    currentCode = code;
}

std::string DexterChuckAudioProcessor::getExampleCode() const
{
    return isEffect() ? readEmbeddedExample("SoftLowPass_ck")
                      : readEmbeddedExample("Welcome_ck");
}

bool DexterChuckAudioProcessor::runCode(const std::string& code,
                                        const std::string& sourcePath,
                                        std::string& error)
{
    if (code.size() > 1024 * 1024)
    {
        error = "This editor supports scripts up to 1 MiB.";
        return false;
    }
    setCurrentCode(code);
    if (!runtime.compile(code, sourcePath, error))
        return false;
    codeStopped.store(false, std::memory_order_release);
    startupError.clear();
    return true;
}

void DexterChuckAudioProcessor::stopCode()
{
    codeStopped.store(true, std::memory_order_release);
    runtime.stop();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DexterChuckAudioProcessor();
}

#pragma once

#include "ChuckRuntime.h"
#include <juce_audio_utils/juce_audio_utils.h>

#include <mutex>
#include <string>

class DexterChuckAudioProcessor final : public juce::AudioProcessor
{
public:
    DexterChuckAudioProcessor();
    ~DexterChuckAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override;
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool isEffect() const noexcept;
    std::string getCurrentCode() const;
    void setCurrentCode(const std::string& code);
    std::string getExampleCode() const;
    bool runCode(const std::string& code, const std::string& sourcePath,
                 std::string& error);
    void stopCode();

private:
    ChuckRuntime runtime;
    mutable std::mutex codeMutex;
    std::string currentCode;
    std::string startupError;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

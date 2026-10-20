#include "PluginProcessor.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>

static void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

int main()
{
    try
    {
        auto processor = std::make_unique<DexterChuckAudioProcessor>();
        require(!processor->acceptsMidi() && !processor->producesMidi(),
                "The plug-in unexpectedly exposes MIDI ports");
        require(processor->getParameters().isEmpty(),
                "The plug-in unexpectedly exposes host automation");
        require(!processor->getCurrentCode().empty(),
                "No built-in ChucK example was loaded");

        processor->prepareToPlay(48000.0, 2048);
        juce::MidiBuffer midi;
        juce::MemoryBlock state;

        if (processor->isEffect())
        {
            processor->stopCode();
            auto code = "adc.chan(0) => dac.chan(0); "
                        "adc.chan(1) => dac.chan(1); "
                        "while(true) 1::samp => now;";
            std::string error;
            require(processor->runCode(code, {}, error), "Effect script compile failed");
            juce::AudioBuffer<float> buffer(2, 2048);
            for (int channel = 0; channel < 2; ++channel)
                buffer.clear(channel, 0, buffer.getNumSamples());
            for (int frame = 0; frame < buffer.getNumSamples(); ++frame)
            {
                buffer.setSample(0, frame, 0.2f);
                buffer.setSample(1, frame, -0.15f);
            }
            processor->processBlock(buffer, midi);
            require(std::abs(buffer.getSample(0, 2047) - 0.2f) < 0.001f,
                    "Effect did not route the left audio input through ChucK");
            require(std::abs(buffer.getSample(1, 2047) + 0.15f) < 0.001f,
                    "Effect did not route the right audio input through ChucK");
        }
        else
        {
            juce::AudioBuffer<float> buffer(2, 2048);
            buffer.clear();
            processor->processBlock(buffer, midi);
            float peak = 0.0f;
            for (int channel = 0; channel < 2; ++channel)
                for (int frame = 0; frame < buffer.getNumSamples(); ++frame)
                {
                    const auto value = buffer.getSample(channel, frame);
                    require(std::isfinite(value), "Instrument rendered non-finite audio");
                    peak = std::max(peak, std::abs(value));
                }
            require(peak > 0.001f, "Instrument produced no audio");
        }

        processor->stopCode();
        juce::AudioBuffer<float> stoppedAudio(2, 512);
        stoppedAudio.clear();
        if (processor->isEffect())
        {
            for (int frame = 0; frame < stoppedAudio.getNumSamples(); ++frame)
            {
                stoppedAudio.setSample(0, frame, 0.1f);
                stoppedAudio.setSample(1, frame, 0.1f);
            }
        }
        processor->processBlock(stoppedAudio, midi);
        require(stoppedAudio.getMagnitude(0, stoppedAudio.getNumSamples()) < 0.00001f
                    && stoppedAudio.getMagnitude(1, stoppedAudio.getNumSamples()) < 0.00001f,
                "Stop did not halt the ChucK audio graph");

        processor->getStateInformation(state);
        require(state.getSize() > 0, "The editor source was not saved with plug-in state");
        const auto savedSource = processor->getCurrentCode();
        processor->setCurrentCode("state round-trip check");
        processor->setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        require(processor->getCurrentCode() == savedSource,
                "The editor source did not restore from plug-in state");
        std::cout << "Processor audio routing, no-MIDI/no-automation setup, "
                     "and source-state round trip passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

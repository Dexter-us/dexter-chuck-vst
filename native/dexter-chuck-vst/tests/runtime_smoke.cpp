#include "ChuckRuntime.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

static void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

int main()
{
    try
    {
        std::string error;
        ChuckRuntime synth;
        require(synth.initialise(48000, 0, 2, error), "Instrument runtime init failed");
        require(synth.compile(
            "SinOsc osc => dac; 0.1 => osc.gain; "
            "while(true) 1::samp => now;", {}, error),
            "Instrument script did not compile");
        std::vector<float> output(1024);
        require(synth.process(nullptr, output.data(), 512), "Instrument render failed");
        float peak = 0.0f;
        for (const auto sample : output)
        {
            require(std::isfinite(sample), "Instrument rendered non-finite audio");
            peak = std::max(peak, std::abs(sample));
        }
        require(peak > 0.05f && peak < 0.2f, "Instrument produced no expected audio");

        ChuckRuntime effect;
        require(effect.initialise(48000, 2, 2, error), "Effect runtime init failed");
        require(effect.compile("adc => dac; while(true) 1::samp => now;", {}, error),
                "Effect passthrough script did not compile");
        std::vector<float> input(1024, 0.25f);
        std::fill(output.begin(), output.end(), 0.0f);
        require(effect.process(input.data(), output.data(), 512), "Effect render failed");
        require(std::abs(output.back() - 0.25f) < 0.001f,
                "Effect did not pass stereo input to its output");

        require(!effect.compile("this is not valid ChucK", {}, error),
                "Invalid ChucK code was accepted");
        require(!error.empty(), "Compiler failure did not return a diagnostic");
        std::cout << "Instrument audio, effect audio passthrough, and compiler "
                     "diagnostics passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class ChuckTokeniser final : public juce::CodeTokeniser
{
public:
    int readNextToken(juce::CodeDocument::Iterator& source) override;
    juce::CodeEditorComponent::ColourScheme getDefaultColourScheme() override;

private:
    static bool isKeyword(const juce::String& word);
    static bool isAudioType(const juce::String& word);
};

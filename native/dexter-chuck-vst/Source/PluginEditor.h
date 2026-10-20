#pragma once

#include "ChuckTokeniser.h"
#include "PluginProcessor.h"

#include <juce_gui_extra/juce_gui_extra.h>

class DexterChuckAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::CodeDocument::Listener
{
public:
    DexterChuckAudioProcessorEditor(DexterChuckAudioProcessor& processor,
                                    const std::string& startupError);
    ~DexterChuckAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void codeDocumentTextInserted(const juce::String& newText, int insertIndex) override;
    void codeDocumentTextDeleted(int startIndex, int endIndex) override;
    void openScript();
    void saveScript();
    void loadFile(const juce::File& file);
    void runScript();
    void loadExample();
    void setStatus(const juce::String& text, bool isError = false);

    DexterChuckAudioProcessor& processor;
    ChuckTokeniser tokeniser;
    juce::CodeDocument document;
    juce::CodeEditorComponent codeEditor;
    juce::Label brandLabel;
    juce::Label modeLabel;
    juce::Label statusLabel;
    juce::TextButton openButton { "Open" };
    juce::TextButton saveButton { "Save" };
    juce::TextButton runButton { "Run ChucK" };
    juce::TextButton stopButton { "Stop" };
    juce::ComboBox exampleBox;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File currentFile;
};

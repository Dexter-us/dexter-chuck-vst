#include "PluginEditor.h"

namespace
{
const juce::Colour background(0xff111820);
const juce::Colour panel(0xff18232d);
const juce::Colour accent(0xffe6b84c);
const juce::Colour muted(0xff9aaab5);
}

DexterChuckAudioProcessorEditor::DexterChuckAudioProcessorEditor(
    DexterChuckAudioProcessor& owner, const std::string& initialError)
    : juce::AudioProcessorEditor(owner),
      processor(owner),
      codeEditor(document, &tokeniser)
{
    setResizable(true, true);
    setResizeLimits(640, 420, 1600, 1200);
    setSize(960, 680);

    document.addListener(this);
    codeEditor.setColour(juce::CodeEditorComponent::backgroundColourId, panel);
    codeEditor.setColour(juce::CodeEditorComponent::highlightColourId,
                         juce::Colour(0xff293743));
    codeEditor.setColour(juce::CodeEditorComponent::defaultTextColourId,
                         juce::Colour(0xffe4e9ee));
    codeEditor.setColour(juce::CaretComponent::caretColourId, accent);
    codeEditor.setFont(juce::Font(juce::FontOptions(15.0f)));
    codeEditor.setTabSize(4, true);
    addAndMakeVisible(codeEditor);

    brandLabel.setText("Dexter U.S. Chuck", juce::dontSendNotification);
    brandLabel.setFont(juce::Font(juce::FontOptions(21.0f, juce::Font::bold)));
    brandLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(brandLabel);

    modeLabel.setText(processor.isEffect() ? "EFFECT" : "INSTRUMENT",
                      juce::dontSendNotification);
    modeLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    modeLabel.setColour(juce::Label::textColourId, accent);
    modeLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(modeLabel);

    for (auto* button : { &openButton, &saveButton, &runButton, &stopButton })
    {
        button->setColour(juce::TextButton::buttonColourId, panel.brighter(0.2f));
        button->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        addAndMakeVisible(*button);
    }
    runButton.setColour(juce::TextButton::buttonColourId, accent.darker(0.2f));
    runButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff101820));
    runButton.setTooltip("Start a new ChucK shred; existing shreds keep running.");
    stopButton.setTooltip("Stop all running ChucK shreds.");
    openButton.onClick = [this] { openScript(); };
    saveButton.onClick = [this] { saveScript(); };
    runButton.onClick = [this] { runScript(); };
    stopButton.onClick = [this] { processor.stopCode(); setStatus("All running shreds stopped."); };

    exampleBox.addItem(processor.isEffect() ? "Soft low-pass example"
                                            : "Welcome instrument example", 1);
    exampleBox.setSelectedId(1, juce::dontSendNotification);
    exampleBox.setColour(juce::ComboBox::backgroundColourId, panel);
    exampleBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    exampleBox.onChange = [this] { loadExample(); };
    addAndMakeVisible(exampleBox);

    statusLabel.setFont(juce::Font(juce::FontOptions(12.0f)));
    statusLabel.setColour(juce::Label::textColourId, muted);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel);

    document.replaceAllContent(juce::String::fromUTF8(processor.getCurrentCode().c_str()));
    setStatus(initialError.empty() ? "Ready — edit the ChucK script and press Run."
                                   : juce::String::fromUTF8(initialError.c_str()),
              !initialError.empty());
}

DexterChuckAudioProcessorEditor::~DexterChuckAudioProcessorEditor()
{
    document.removeListener(this);
}

void DexterChuckAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(background);
    graphics.setColour(accent);
    graphics.fillRect(0, 0, getWidth(), 3);
    graphics.setColour(panel);
    graphics.fillRect(0, 54, getWidth(), 1);
}

void DexterChuckAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(18);
    auto header = area.removeFromTop(44);
    brandLabel.setBounds(header.removeFromLeft(280));
    modeLabel.setBounds(header.removeFromRight(150));
    area.removeFromTop(12);

    auto controls = area.removeFromTop(34);
    openButton.setBounds(controls.removeFromLeft(76));
    controls.removeFromLeft(8);
    saveButton.setBounds(controls.removeFromLeft(76));
    controls.removeFromLeft(8);
    runButton.setBounds(controls.removeFromLeft(112));
    controls.removeFromLeft(8);
    stopButton.setBounds(controls.removeFromLeft(76));
    controls.removeFromLeft(14);
    exampleBox.setBounds(controls.removeFromLeft(240));
    area.removeFromTop(10);

    auto status = area.removeFromBottom(24);
    statusLabel.setBounds(status);
    area.removeFromBottom(8);
    codeEditor.setBounds(area);
}

void DexterChuckAudioProcessorEditor::codeDocumentTextInserted(
    const juce::String&, int)
{
    processor.setCurrentCode(document.getAllContent().toStdString());
}

void DexterChuckAudioProcessorEditor::codeDocumentTextDeleted(int, int)
{
    processor.setCurrentCode(document.getAllContent().toStdString());
}

void DexterChuckAudioProcessorEditor::openScript()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Open a ChucK script", currentFile, "*.ck");
    juce::Component::SafePointer<DexterChuckAudioProcessorEditor> safeThis(this);
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode
                                 | juce::FileBrowserComponent::canSelectFiles,
                             [safeThis](const juce::FileChooser& chooser)
                             {
                                 if (safeThis)
                                     safeThis->loadFile(chooser.getResult());
                             });
}

void DexterChuckAudioProcessorEditor::saveScript()
{
    if (currentFile.existsAsFile())
    {
        const auto saved = currentFile.replaceWithText(document.getAllContent());
        setStatus(saved ? "Script saved." : "Could not save the script.", !saved);
        return;
    }

    fileChooser = std::make_unique<juce::FileChooser>(
        "Save ChucK script", juce::File::getSpecialLocation(
            juce::File::userDocumentsDirectory).getChildFile("Untitled.ck"), "*.ck");
    juce::Component::SafePointer<DexterChuckAudioProcessorEditor> safeThis(this);
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode
                                 | juce::FileBrowserComponent::canSelectFiles
                                 | juce::FileBrowserComponent::warnAboutOverwriting,
                             [safeThis](const juce::FileChooser& chooser)
                             {
                                 if (safeThis)
                                 {
                                     const auto file = chooser.getResult();
                                     if (file != juce::File())
                                     {
                                         const auto outputFile = file.hasFileExtension(".ck")
                                             ? file : file.withFileExtension(".ck");
                                         const auto saved = outputFile.replaceWithText(
                                             safeThis->document.getAllContent());
                                         if (saved)
                                             safeThis->currentFile = outputFile;
                                         safeThis->setStatus(saved ? "Script saved."
                                                                   : "Could not save the script.",
                                                             !saved);
                                     }
                                 }
                             });
}

void DexterChuckAudioProcessorEditor::loadFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return;
    const auto source = file.loadFileAsString();
    if (source.getNumBytesAsUTF8() > 1024 * 1024)
    {
        setStatus("Script files must be 1 MiB or smaller.", true);
        return;
    }
    currentFile = file;
    document.replaceAllContent(source);
    processor.setCurrentCode(source.toStdString());
    setStatus("Loaded " + file.getFileName());
}

void DexterChuckAudioProcessorEditor::runScript()
{
    std::string error;
    const auto sourcePath = currentFile.existsAsFile()
        ? currentFile.getFullPathName().toStdString() : std::string{};
    if (processor.runCode(document.getAllContent().toStdString(), sourcePath, error))
        setStatus("ChucK script is running.");
    else
        setStatus(juce::String::fromUTF8(error.c_str()), true);
}

void DexterChuckAudioProcessorEditor::loadExample()
{
    const auto example = processor.getExampleCode();
    if (example.empty())
    {
        setStatus("The built-in example could not be loaded.", true);
        return;
    }
    currentFile = juce::File();
    const auto source = juce::String::fromUTF8(example.c_str());
    document.replaceAllContent(source);
    processor.setCurrentCode(example);
    setStatus("Example loaded. Press Run ChucK to execute it.");
}

void DexterChuckAudioProcessorEditor::setStatus(const juce::String& text, bool isError)
{
    statusLabel.setText(text, juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId,
                          isError ? juce::Colour(0xffff6470) : muted);
}

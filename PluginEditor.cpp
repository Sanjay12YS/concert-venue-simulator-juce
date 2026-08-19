/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ConcertVenueAudioProcessorEditor::ConcertVenueAudioProcessorEditor (ConcertVenueAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // --- Room Size Knob ---
    roomSizeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    roomSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(roomSizeSlider);
    roomSizeLabel.setText("Room Size", juce::dontSendNotification);
    roomSizeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(roomSizeLabel);
    roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "roomSize", roomSizeSlider);

    // --- Damping Knob ---
    dampingSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(dampingSlider);
    dampingLabel.setText("Damping", juce::dontSendNotification);
    dampingLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dampingLabel);
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "damping", dampingSlider);

    // --- Width Knob ---
    widthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(widthSlider);
    widthLabel.setText("Width", juce::dontSendNotification);
    widthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(widthLabel);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "width", widthSlider);

    // --- Wet Level Knob ---
    wetLevelSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wetLevelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(wetLevelSlider);
    wetLevelLabel.setText("Wet Level", juce::dontSendNotification);
    wetLevelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wetLevelLabel);
    wetLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "wetLevel", wetLevelSlider);

    // --- Dry Level Knob ---
    dryLevelSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dryLevelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(dryLevelSlider);
    dryLevelLabel.setText("Dry Level", juce::dontSendNotification);
    dryLevelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(dryLevelLabel);
    dryLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "dryLevel", dryLevelSlider);

    // --- LPF Cutoff Knob ---
    lpfCutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    lpfCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(lpfCutoffSlider);
    lpfCutoffLabel.setText("LPF Cutoff", juce::dontSendNotification);
    lpfCutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lpfCutoffLabel);
    lpfCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "lpfCutoff", lpfCutoffSlider);

    // --- Preset Buttons ---
    clubButton.setButtonText("Club");
    addAndMakeVisible(clubButton);
    clubButton.onClick = [this]()
    {
        // Small jazz club - dry, intimate, bright
        audioProcessor.parameters.getParameter("roomSize")->setValueNotifyingHost(0.2f);
        audioProcessor.parameters.getParameter("damping")->setValueNotifyingHost(0.7f);
        audioProcessor.parameters.getParameter("wetLevel")->setValueNotifyingHost(0.3f);
        audioProcessor.parameters.getParameter("dryLevel")->setValueNotifyingHost(0.7f);
        audioProcessor.parameters.getParameter("width")->setValueNotifyingHost(0.4f);
        audioProcessor.parameters.getParameter("lpfCutoff")->setValueNotifyingHost(0.74f);
    };

    theatreButton.setButtonText("Theatre");
    addAndMakeVisible(theatreButton);
    theatreButton.onClick = [this]()
    {
        // Concert theatre - balanced, medium space
        audioProcessor.parameters.getParameter("roomSize")->setValueNotifyingHost(0.55f);
        audioProcessor.parameters.getParameter("damping")->setValueNotifyingHost(0.5f);
        audioProcessor.parameters.getParameter("wetLevel")->setValueNotifyingHost(0.5f);
        audioProcessor.parameters.getParameter("dryLevel")->setValueNotifyingHost(0.5f);
        audioProcessor.parameters.getParameter("width")->setValueNotifyingHost(0.7f);
        audioProcessor.parameters.getParameter("lpfCutoff")->setValueNotifyingHost(0.42f);
    };

    arenaButton.setButtonText("Arena");
    addAndMakeVisible(arenaButton);
    arenaButton.onClick = [this]()
    {
        // Large arena - big and wide but performer still clear
        audioProcessor.parameters.getParameter("roomSize")->setValueNotifyingHost(0.75f);
        audioProcessor.parameters.getParameter("damping")->setValueNotifyingHost(0.4f);
        audioProcessor.parameters.getParameter("wetLevel")->setValueNotifyingHost(0.80f);
        audioProcessor.parameters.getParameter("dryLevel")->setValueNotifyingHost(0.47f);
        audioProcessor.parameters.getParameter("width")->setValueNotifyingHost(0.85f);
        audioProcessor.parameters.getParameter("lpfCutoff")->setValueNotifyingHost(0.3f);
    };

    // --- Load Audio File Button ---
    // Opens a file picker so the user can load a wav or mp3 file
    loadFileButton.setButtonText("Load Audio File");
    addAndMakeVisible(loadFileButton);
    loadFileButton.onClick = [this]()
    {
        // Open a file chooser filtered to audio files only
        auto chooser = std::make_shared<juce::FileChooser>(
            "Select an audio file",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.wav;*.mp3;*.aiff;*.flac"
        );

        chooser->launchAsync(juce::FileBrowserComponent::openMode |
                             juce::FileBrowserComponent::canSelectFiles,
                             [this, chooser](const juce::FileChooser& fc)
                             {
                                 auto file = fc.getResult();
                                 if (file.existsAsFile())
                                 {
                                     // Load the file into the processor
                                     audioProcessor.loadAudioFile(file);

                                     // Show the filename on the label
                                     fileNameLabel.setText(file.getFileName(),
                                                           juce::dontSendNotification);

                                     // Turn off white noise when a file is loaded
                                     audioProcessor.isWhiteNoiseOn = false;
                                     whiteNoiseButton.setButtonText("White Noise: OFF");
                                 }
                             });
    };

    // --- White Noise Button ---
    // Toggles white noise on and off as a test signal
    whiteNoiseButton.setButtonText("White Noise: OFF");
    addAndMakeVisible(whiteNoiseButton);
    whiteNoiseButton.onClick = [this]()
    {
        // Flip the white noise state in the processor
        audioProcessor.isWhiteNoiseOn = !audioProcessor.isWhiteNoiseOn;

        // Update button text to show current state
        if (audioProcessor.isWhiteNoiseOn)
        {
            whiteNoiseButton.setButtonText("White Noise: ON");
            // Turn off file playback when white noise is on
            audioProcessor.isFileLoaded = false;
            fileNameLabel.setText("No file loaded", juce::dontSendNotification);
        }
        else
        {
            whiteNoiseButton.setButtonText("White Noise: OFF");
        }
    };

    // --- File Name Label ---
    // Shows which file is currently loaded
    fileNameLabel.setText("No file loaded", juce::dontSendNotification);
    fileNameLabel.setJustificationType(juce::Justification::centred);
    fileNameLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(fileNameLabel);

    // Set the plugin window size
    setSize(500, 460);
}

ConcertVenueAudioProcessorEditor::~ConcertVenueAudioProcessorEditor()
{
}

//==============================================================================
void ConcertVenueAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dark background
    g.fillAll(juce::Colour(30, 30, 30));

    // Title text at the top
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawFittedText("Concert Venue Simulator",
                     0, 10, getWidth(), 30,
                     juce::Justification::centred, 1);

    // Subtle divider line under the title
    g.setColour(juce::Colour(80, 80, 80));
    g.drawLine(20, 45, getWidth() - 20, 45, 1.0f);

    // Divider line above the bottom buttons
    g.drawLine(20, 370, getWidth() - 20, 370, 1.0f);
}

//==============================================================================
void ConcertVenueAudioProcessorEditor::resized()
{
    // --- Preset buttons row ---
    int buttonY    = 55;
    int buttonH    = 30;
    int buttonW    = 120;
    int buttonGap  = 20;
    int totalBtnW  = (buttonW * 3) + (buttonGap * 2);
    int btnStartX  = (getWidth() - totalBtnW) / 2;

    clubButton.setBounds   (btnStartX,                            buttonY, buttonW, buttonH);
    theatreButton.setBounds(btnStartX + buttonW + buttonGap,      buttonY, buttonW, buttonH);
    arenaButton.setBounds  (btnStartX + (buttonW + buttonGap) * 2, buttonY, buttonW, buttonH);

    // --- Knobs ---
    int knobSize   = 90;
    int labelH     = 20;
    int row1Y      = 110;
    int row2Y      = 240;
    int knobGap    = 20;
    int totalKnobW = (knobSize * 3) + (knobGap * 2);
    int knobStartX = (getWidth() - totalKnobW) / 2;

    // Row 1 - Room Size, Damping, Width
    roomSizeSlider.setBounds(knobStartX,                             row1Y, knobSize, knobSize);
    roomSizeLabel.setBounds (knobStartX,                             row1Y + knobSize, knobSize, labelH);

    dampingSlider.setBounds (knobStartX + knobSize + knobGap,        row1Y, knobSize, knobSize);
    dampingLabel.setBounds  (knobStartX + knobSize + knobGap,        row1Y + knobSize, knobSize, labelH);

    widthSlider.setBounds   (knobStartX + (knobSize + knobGap) * 2,  row1Y, knobSize, knobSize);
    widthLabel.setBounds    (knobStartX + (knobSize + knobGap) * 2,  row1Y + knobSize, knobSize, labelH);

    // Row 2 - Wet Level, Dry Level, LPF Cutoff
    wetLevelSlider.setBounds (knobStartX,                             row2Y, knobSize, knobSize);
    wetLevelLabel.setBounds  (knobStartX,                             row2Y + knobSize, knobSize, labelH);

    dryLevelSlider.setBounds (knobStartX + knobSize + knobGap,        row2Y, knobSize, knobSize);
    dryLevelLabel.setBounds  (knobStartX + knobSize + knobGap,        row2Y + knobSize, knobSize, labelH);

    lpfCutoffSlider.setBounds(knobStartX + (knobSize + knobGap) * 2,  row2Y, knobSize, knobSize);
    lpfCutoffLabel.setBounds (knobStartX + (knobSize + knobGap) * 2,  row2Y + knobSize, knobSize, labelH);

    // --- Bottom section - Load File and White Noise ---
    int bottomY    = 380;
    int bottomBtnW = 180;
    int bottomBtnH = 30;
    int bottomGap  = 20;
    int totalBotW  = (bottomBtnW * 2) + bottomGap;
    int botStartX  = (getWidth() - totalBotW) / 2;

    loadFileButton.setBounds  (botStartX,                      bottomY, bottomBtnW, bottomBtnH);
    whiteNoiseButton.setBounds(botStartX + bottomBtnW + bottomGap, bottomY, bottomBtnW, bottomBtnH);

    // File name label sits below the buttons
    fileNameLabel.setBounds(0, bottomY + bottomBtnH + 10, getWidth(), 20);
}

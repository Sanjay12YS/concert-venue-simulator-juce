/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// This is the GUI of the plugin - everything the user sees and interacts with
// The editor talks to the processor through the audioProcessor reference
class ConcertVenueAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ConcertVenueAudioProcessorEditor (ConcertVenueAudioProcessor&);
    ~ConcertVenueAudioProcessorEditor() override;

    //==============================================================================
    // paint() draws the background and title
    void paint (juce::Graphics&) override;

    // resized() positions all the knobs and buttons
    void resized() override;

private:
    //==============================================================================
    // Reference to the processor so we can read/write parameters
    ConcertVenueAudioProcessor& audioProcessor;

    //==============================================================================
    // Six knobs - one for each parameter
    juce::Slider roomSizeSlider;
    juce::Slider dampingSlider;
    juce::Slider widthSlider;
    juce::Slider wetLevelSlider;
    juce::Slider dryLevelSlider;
    juce::Slider lpfCutoffSlider;

    // Labels that sit below each knob
    juce::Label roomSizeLabel;
    juce::Label dampingLabel;
    juce::Label widthLabel;
    juce::Label wetLevelLabel;
    juce::Label dryLevelLabel;
    juce::Label lpfCutoffLabel;

    // Preset buttons - clicking these sets all sliders at once
    juce::TextButton clubButton;
    juce::TextButton theatreButton;
    juce::TextButton arenaButton;

    // Load audio file button - opens a file picker
    juce::TextButton loadFileButton;

    // White noise toggle button - turns white noise on and off
    juce::TextButton whiteNoiseButton;

    // Label showing the name of the currently loaded file
    juce::Label fileNameLabel;

    // Attachments connect the sliders to the parameter tree
    // When the slider moves, the parameter updates automatically
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpfCutoffAttachment;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertVenueAudioProcessorEditor)
};

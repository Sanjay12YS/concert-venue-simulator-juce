/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
// Main processor class - this is the brain of the plugin
// All DSP logic lives here, the editor (GUI) is separate
class ConcertVenueAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    // Constructor and Destructor
    ConcertVenueAudioProcessor();
    ~ConcertVenueAudioProcessor() override;

    //==============================================================================
    // Called once before audio starts - set up reverb, filters, buffers here
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    // Called when plugin stops - clean up any resources
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    // Checks if the plugin supports the current channel layout (mono/stereo)
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    // This is where all the audio processing happens - runs in real time
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    // Creates the GUI editor window
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    // This is an audio effect, not a MIDI instrument
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    // Reverb has a tail so sound continues after input stops
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Program/preset management - kept simple for now
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    // Saves and loads plugin state so DAW can recall our settings
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter manager - public so the editor can attach sliders to it
    juce::AudioProcessorValueTreeState parameters;

    // --- Audio File Loading ---
    // Loads an audio file from disk and stores it in a buffer
    void loadAudioFile(const juce::File& file);

    // --- White Noise ---
    // Toggle white noise on or off
    bool isWhiteNoiseOn = false;
    
    // True when a file is loaded and ready to play
    bool isFileLoaded = false;


private:
    //==============================================================================
    // juce::Reverb implements the Schroeder algorithm internally
    // reverbParams holds all the values we send to it each block
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    // Separate filter for left and right channels
    // IIR is efficient for real time - simulates air absorbing high frequencies
    juce::IIRFilter lowPassLeft;
    juce::IIRFilter lowPassRight;

    // Stores a copy of the original signal before any processing
    // Used for the wet/dry blend at the end of the signal chain
    juce::AudioBuffer<float> dryBuffer;

    // --- Audio File Playback ---
    // Handles reading and formatting audio files from disk
    juce::AudioFormatManager formatManager;

    // Stores the loaded audio file in memory ready for playback
    std::unique_ptr<juce::AudioBuffer<float>> fileBuffer;

    // Tracks where we are in the loaded audio file
    int filePlaybackPosition = 0;


    // --- White Noise ---
    // Random number generator for white noise generation
    juce::Random random;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertVenueAudioProcessor)
};

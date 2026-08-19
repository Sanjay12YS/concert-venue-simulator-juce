/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Constructor - setting up all the parameters the user can control
ConcertVenueAudioProcessor::ConcertVenueAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this, nullptr, "Parameters", {
        // Controls how big the room feels
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("roomSize", 1), "Room Size", 0.0f, 1.0f, 0.5f),
        // How much the walls/audience absorb high frequencies
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("damping", 1), "Damping", 0.0f, 1.0f, 0.5f),
        // How much of the reverb effect we hear
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("wetLevel", 1), "Wet Level", 0.0f, 1.0f, 0.5f),
        // How much of the original signal we keep
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("dryLevel", 1), "Dry Level", 0.0f, 1.0f, 0.5f),
        // How wide the stereo image feels
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("width", 1), "Width", 0.0f, 1.0f, 0.5f),
        // Low pass filter cutoff - simulates air absorbing high frequencies
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("lpfCutoff", 1), "LPF Cutoff", 1000.0f, 20000.0f, 8000.0f)
    })
{
    // Register basic audio formats so we can load wav and mp3 files
    formatManager.registerBasicFormats();
}

ConcertVenueAudioProcessor::~ConcertVenueAudioProcessor()
{
}

const juce::String ConcertVenueAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

// This is an audio effect plugin, not a MIDI instrument
bool ConcertVenueAudioProcessor::acceptsMidi() const { return false; }
bool ConcertVenueAudioProcessor::producesMidi() const { return false; }
bool ConcertVenueAudioProcessor::isMidiEffect() const { return false; }
double ConcertVenueAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ConcertVenueAudioProcessor::getNumPrograms() { return 1; }
int ConcertVenueAudioProcessor::getCurrentProgram() { return 0; }
void ConcertVenueAudioProcessor::setCurrentProgram(int index) {}
const juce::String ConcertVenueAudioProcessor::getProgramName(int index) { return {}; }
void ConcertVenueAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

// Called once before playback starts - initialise everything here
void ConcertVenueAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Tell reverb what sample rate we're running at so timing is correct
    reverb.setSampleRate(sampleRate);
    reverb.reset();

    // Reset filters so there's no leftover state from before
    lowPassLeft.reset();
    lowPassRight.reset();

    // Set up the dry buffer to store the original signal before processing
    dryBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    dryBuffer.clear();
}

// Clean up when plugin stops
void ConcertVenueAudioProcessor::releaseResources()
{
    reverb.reset();
    lowPassLeft.reset();
    lowPassRight.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ConcertVenueAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
  #endif
}
#endif

// --- Load Audio File ---
// This function is called from the GUI when the user picks a file
// It reads the file from disk and stores it in fileBuffer ready for playback
void ConcertVenueAudioProcessor::loadAudioFile(const juce::File& file)
{
    // Create a reader for the selected file
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        // Create a new buffer big enough to hold the whole file
        auto newBuffer = std::make_unique<juce::AudioBuffer<float>>(
            reader->numChannels,
            (int)reader->lengthInSamples
        );

        // Read the entire file into the buffer
        reader->read(newBuffer.get(), 0,
                     (int)reader->lengthInSamples, 0, true, true);

        // Store the buffer and reset playback position to the start
        fileBuffer = std::move(newBuffer);
        filePlaybackPosition = 0;
        isFileLoaded = true;

        delete reader;
    }
}

// This is where all the audio processing happens - called repeatedly with chunks of audio
void ConcertVenueAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Read the current slider values from the user
    float roomSize  = *parameters.getRawParameterValue("roomSize");
    float damping   = *parameters.getRawParameterValue("damping");
    float wetLevel  = *parameters.getRawParameterValue("wetLevel");
    float dryLevel  = *parameters.getRawParameterValue("dryLevel");
    float width     = *parameters.getRawParameterValue("width");
    float lpfCutoff = *parameters.getRawParameterValue("lpfCutoff");

    int numSamples  = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    // --- White Noise ---
    // If white noise is on, fill the buffer with random samples
    // This is useful for testing the reverb and filter effect clearly
    if (isWhiteNoiseOn)
    {
        for (int ch = 0; ch < numChannels; ch++)
            for (int i = 0; i < numSamples; i++)
                buffer.getWritePointer(ch)[i] = random.nextFloat() * 0.2f - 0.1f;
    }
    // --- Audio File Playback ---
    // If a file is loaded, mix it into the buffer
    else if (isFileLoaded && fileBuffer != nullptr)
    {
        int fileChannels = fileBuffer->getNumChannels();
        int fileSamples  = fileBuffer->getNumSamples();

        for (int ch = 0; ch < numChannels; ch++)
        {
            // If file is mono, always read from channel 0
            int fileChannel = (ch < fileChannels) ? ch : 0;

            for (int i = 0; i < numSamples; i++)
            {
                // Loop the file when it reaches the end
                if (filePlaybackPosition + i >= fileSamples)
                    filePlaybackPosition = -(i);

                buffer.getWritePointer(ch)[i] =
                    fileBuffer->getReadPointer(fileChannel)[filePlaybackPosition + i];
            }
        }

        // Move playback position forward by one block
        filePlaybackPosition += numSamples;

        // Loop back to start when file ends
        if (filePlaybackPosition >= fileBuffer->getNumSamples())
            filePlaybackPosition = 0;
    }

    // Save a copy of the original signal before we do anything to it
    dryBuffer.makeCopyOf(buffer);

    // --- Low Pass Filter ---
    // Simulates how air absorbs high frequencies over distance
    // Bigger room = lower cutoff = darker, more muffled sound
    auto lpfCoeffs = juce::IIRCoefficients::makeLowPass(getSampleRate(), lpfCutoff);
    lowPassLeft.setCoefficients(lpfCoeffs);
    lowPassRight.setCoefficients(lpfCoeffs);

    lowPassLeft.processSamples(buffer.getWritePointer(0), numSamples);
    if (numChannels > 1)
        lowPassRight.processSamples(buffer.getWritePointer(1), numSamples);

    // --- Stereo Width using Mid/Side processing ---
    // Mid = centre sound (performer), Side = spread (room ambience)
    // Scaling the side channel controls how wide the venue feels
    if (numChannels >= 2)
    {
        for (int i = 0; i < numSamples; i++)
        {
            float left  = buffer.getReadPointer(0)[i];
            float right = buffer.getReadPointer(1)[i];

            // Encode to Mid/Side
            float mid  = (left + right) * 0.5f;
            float side = (left - right) * 0.5f;

            // Scale the side by the width parameter
            side *= width;

            // Decode back to Left/Right
            buffer.getWritePointer(0)[i] = mid + side;
            buffer.getWritePointer(1)[i] = mid - side;
        }
    }

    // --- Reverb ---
    // juce::Reverb uses the Schroeder algorithm internally
    // Room size controls decay time, damping controls how bright the reverb is
    reverbParams.roomSize = roomSize;
    reverbParams.damping  = damping;
    reverbParams.wetLevel = wetLevel;
    reverbParams.dryLevel = dryLevel;
    reverbParams.width    = width;
    reverb.setParameters(reverbParams);

    if (numChannels >= 2)
        reverb.processStereo(buffer.getWritePointer(0),
                             buffer.getWritePointer(1),
                             numSamples);
    else
        reverb.processMono(buffer.getWritePointer(0),
                           numSamples);
}

bool ConcertVenueAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ConcertVenueAudioProcessor::createEditor()
{
    return new ConcertVenueAudioProcessorEditor(*this);
}

// Save plugin state so DAW can recall settings
void ConcertVenueAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

// Reload saved settings when DAW reopens the plugin
void ConcertVenueAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ConcertVenueAudioProcessor();
}

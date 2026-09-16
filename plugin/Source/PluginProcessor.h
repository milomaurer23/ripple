#pragma once
#include <JuceHeader.h>
#include "Parameters.h"
#include "RippleGenerator.h"

class RippleAudioProcessor final : public juce::AudioProcessor
{
public:
    RippleAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // For the editor: what the generator is currently doing, so the curve view can draw the
    // exact same shape being applied to the audio.
    RippleParams getCurrentRippleParams() const { return lastRippleParams; }
    double getGeneratorPhaseSeconds() const { return leftGenerator.getPhaseSeconds(); }

    juce::AudioProcessorValueTreeState state;

private:
    RippleParams buildRippleParams (double sampleRate, double hostBpm, double ppqPosition, bool isPlaying);

    std::atomic<float>* pRateHz = nullptr;
    std::atomic<float>* pSyncDiv = nullptr;
    std::atomic<float>* pSyncMode = nullptr;
    std::atomic<float>* pDepth = nullptr;
    std::atomic<float>* pRipples = nullptr;
    std::atomic<float>* pDecay = nullptr;
    std::atomic<float>* pTension = nullptr;
    std::atomic<float>* pStereoOffset = nullptr;
    std::atomic<float>* pMix = nullptr;
    std::atomic<float>* pOutputGain = nullptr;

    RippleGenerator leftGenerator;
    RippleParams lastRippleParams;

    std::vector<float> gainHistory;
    int gainHistoryWrite = 0;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RippleAudioProcessor)
};

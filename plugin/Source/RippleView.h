#pragma once
#include <JuceHeader.h>
#include "RippleGenerator.h"

class RippleAudioProcessor;

// Draws the actual gain-vs-time curve the generator is applying (sampled from the same
// RippleGenerator::computeGain used by the DSP), with a marker showing the live playback phase.
class RippleView final : public juce::Component, private juce::Timer
{
public:
    explicit RippleView (RippleAudioProcessor& processorIn);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }

    RippleAudioProcessor& processor;
};

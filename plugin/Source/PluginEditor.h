#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RippleView.h"

class RippleAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit RippleAudioProcessorEditor (RippleAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    Knob& addKnob (const char* paramId, const juce::String& labelText);

    RippleAudioProcessor& audioProcessor;
    RippleView rippleView;

    juce::ToggleButton syncButton { "Tempo Sync" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment;

    juce::ComboBox syncDivBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> syncDivAttachment;

    juce::OwnedArray<Knob> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RippleAudioProcessorEditor)
};

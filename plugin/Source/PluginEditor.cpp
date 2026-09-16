#include "PluginEditor.h"

RippleAudioProcessorEditor::RippleAudioProcessorEditor (RippleAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), rippleView (p)
{
    addAndMakeVisible (rippleView);

    addAndMakeVisible (syncButton);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.state, ParamIDs::syncMode, syncButton);

    addAndMakeVisible (syncDivBox);
    juce::StringArray divNames;
    for (auto& d : syncDivisions()) divNames.add (d.name);
    syncDivBox.addItemList (divNames, 1);
    syncDivAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        audioProcessor.state, ParamIDs::syncDiv, syncDivBox);

    addKnob (ParamIDs::rateHz, "Rate");
    addKnob (ParamIDs::depth, "Depth");
    addKnob (ParamIDs::ripples, "Ripples");
    addKnob (ParamIDs::decay, "Decay");
    addKnob (ParamIDs::tension, "Tension");
    addKnob (ParamIDs::stereoOffset, "Stereo");
    addKnob (ParamIDs::mix, "Mix");
    addKnob (ParamIDs::outputGain, "Output");

    setSize (480, 380);
}

RippleAudioProcessorEditor::Knob& RippleAudioProcessorEditor::addKnob (const char* paramId, const juce::String& labelText)
{
    auto* knob = knobs.add (new Knob());
    knob->label.setText (labelText, juce::dontSendNotification);
    knob->label.setJustificationType (juce::Justification::centred);
    knob->label.attachToComponent (&knob->slider, false);

    addAndMakeVisible (knob->slider);
    addAndMakeVisible (knob->label);

    knob->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.state, paramId, knob->slider);

    return *knob;
}

void RippleAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (0x0c, 0x10, 0x14));
}

void RippleAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (12);

    rippleView.setBounds (bounds.removeFromTop (150));
    bounds.removeFromTop (28); // room for the row of knob labels above the sync row

    auto syncRow = bounds.removeFromTop (28);
    syncButton.setBounds (syncRow.removeFromLeft (110));
    syncDivBox.setBounds (syncRow.removeFromLeft (90).reduced (0, 2));

    bounds.removeFromTop (10);

    const int numKnobs = knobs.size();
    const int knobWidth = bounds.getWidth() / numKnobs;
    for (int i = 0; i < numKnobs; ++i)
        knobs.getUnchecked (i)->slider.setBounds (bounds.removeFromLeft (knobWidth).reduced (4));
}

#pragma once
#include <JuceHeader.h>

namespace ParamIDs
{
    constexpr auto rateHz       = "rateHz";
    constexpr auto syncDiv      = "syncDiv";
    constexpr auto syncMode     = "syncMode";
    constexpr auto depth        = "depth";
    constexpr auto ripples      = "ripples";
    constexpr auto decay        = "decay";
    constexpr auto tension      = "tension";
    constexpr auto stereoOffset = "stereoOffset";
    constexpr auto mix          = "mix";
    constexpr auto outputGain   = "outputGain";
}

// Sync division choices, in quarter-note beats per trigger period (host ppqPosition is in
// quarter notes, so this is what a division needs to be expressed in to read the playhead).
struct SyncDivision { const char* name; double beats; };
inline const std::vector<SyncDivision>& syncDivisions()
{
    static const std::vector<SyncDivision> divs = {
        { "1/1",   4.0 },
        { "1/2.",  3.0 },
        { "1/2",   2.0 },
        { "1/4.",  1.5 },
        { "1/4",   1.0 },
        { "1/4T",  1.0 * 2.0 / 3.0 },
        { "1/8.",  0.75 },
        { "1/8",   0.5 },
        { "1/8T",  0.5 * 2.0 / 3.0 },
        { "1/16.", 0.375 },
        { "1/16",  0.25 },
        { "1/16T", 0.25 * 2.0 / 3.0 },
        { "1/32",  0.125 },
    };
    return divs;
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createRippleParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::rateHz, "Rate",
        NormalisableRange<float> (0.05f, 20.0f, 0.0f, 0.35f), 2.0f, "Hz"));

    juce::StringArray divNames;
    for (auto& d : syncDivisions()) divNames.add (d.name);
    params.push_back (std::make_unique<AudioParameterChoice> (ParamIDs::syncDiv, "Sync Division", divNames, 4));

    params.push_back (std::make_unique<AudioParameterBool> (ParamIDs::syncMode, "Tempo Sync", false));

    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::depth, "Depth",
        NormalisableRange<float> (0.0f, 1.0f), 0.7f));

    params.push_back (std::make_unique<AudioParameterInt> (ParamIDs::ripples, "Ripples", 1, 8, 3));

    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::decay, "Decay",
        NormalisableRange<float> (0.0f, 1.0f), 0.5f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::tension, "Tension",
        NormalisableRange<float> (0.0f, 1.0f), 0.3f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::stereoOffset, "Stereo Offset",
        NormalisableRange<float> (0.0f, 50.0f), 8.0f, "ms"));

    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::mix, "Mix",
        NormalisableRange<float> (0.0f, 1.0f), 1.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::outputGain, "Output",
        NormalisableRange<float> (-12.0f, 12.0f), 0.0f, "dB"));

    return { params.begin(), params.end() };
}

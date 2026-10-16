#pragma once
#include <JuceHeader.h>

// Generated code: keep the plugin's strict warning set for our own sources only.
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wextra-semi", "-Wunused-parameter", "-Wfloat-equal",
                                     "-Wshadow", "-Wsign-conversion", "-Wconversion",
                                     "-Wimplicit-float-conversion", "-Wold-style-cast",
                                     "-Wzero-as-null-pointer-constant", "-Wcast-align")
#include "RippleDSP.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

// Runs Ripple's audio path, which lives in plugin/Faust/Ripple.dsp (compiled to RippleDSP.h by
// scripts/faust.sh). This class only feeds the Faust controls and hands it the buffers.
struct RippleEngineSettings
{
    float depth = 0.7f;
    int ripples = 3;
    float decay = 0.5f;
    float tension = 0.3f;
    double periodSeconds = 0.5;      // 0 disables retriggering (gain stays at unity)
    bool resync = false;             // tempo-synced: lock the clock to syncPhaseSeconds this block
    double syncPhaseSeconds = 0.0;
    float stereoOffsetMs = 8.0f;
    float mix = 1.0f;
    float outputGainDb = 0.0f;
};

class RippleEngine
{
public:
    RippleEngine();

    void prepare (double sampleRate, int maxBlockSize);
    void process (juce::AudioBuffer<float>& buffer, const RippleEngineSettings& settings);

    // Seconds since the last trigger, as of the end of the last processed block. Read by the
    // editor's curve view from the message thread.
    double getPhaseSeconds() const { return phaseSeconds.load(); }

private:
    struct Zones : ripplefaust::UI
    {
        FAUSTFLOAT* depth = nullptr;
        FAUSTFLOAT* ripples = nullptr;
        FAUSTFLOAT* decay = nullptr;
        FAUSTFLOAT* tension = nullptr;
        FAUSTFLOAT* period = nullptr;
        FAUSTFLOAT* stereoOffset = nullptr;
        FAUSTFLOAT* mix = nullptr;
        FAUSTFLOAT* outputGain = nullptr;
        FAUSTFLOAT* synced = nullptr;
        FAUSTFLOAT* syncPhase = nullptr;
        FAUSTFLOAT* blockTick = nullptr;
        FAUSTFLOAT* phaseOut = nullptr;

        FAUSTFLOAT** find (const char* label);
        void bind (const char* label, FAUSTFLOAT* zone);

        void openTabBox (const char*) override {}
        void openHorizontalBox (const char*) override {}
        void openVerticalBox (const char*) override {}
        void closeBox() override {}
        void addButton (const char* l, FAUSTFLOAT* z) override { bind (l, z); }
        void addCheckButton (const char* l, FAUSTFLOAT* z) override { bind (l, z); }
        void addVerticalSlider (const char* l, FAUSTFLOAT* z, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) override { bind (l, z); }
        void addHorizontalSlider (const char* l, FAUSTFLOAT* z, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) override { bind (l, z); }
        void addNumEntry (const char* l, FAUSTFLOAT* z, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) override { bind (l, z); }
        void addHorizontalBargraph (const char* l, FAUSTFLOAT* z, FAUSTFLOAT, FAUSTFLOAT) override { bind (l, z); }
        void addVerticalBargraph (const char* l, FAUSTFLOAT* z, FAUSTFLOAT, FAUSTFLOAT) override { bind (l, z); }
        void addSoundfile (const char*, const char*, ripplefaust::Soundfile**) override {}
    };

    ripplefaust::RippleDSP faust;
    Zones zones;
    std::array<FAUSTFLOAT, 12> unboundZones {};
    bool blockTick = false;
    std::atomic<double> phaseSeconds { 0.0 };

    // Mono layouts: Faust always runs stereo, so the right channel reads and writes here.
    juce::AudioBuffer<float> spareChannel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RippleEngine)
};

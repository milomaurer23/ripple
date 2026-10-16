#include "RippleEngine.h"

RippleEngine::RippleEngine()
{
    faust.buildUserInterface (&zones);

    // Faust drops controls the .dsp no longer uses, and a renamed one never binds. Either way the
    // slot would stay null and crash the audio thread, so park it on a spare instead.
    int spare = 0;
    for (auto** zone : { &zones.depth, &zones.ripples, &zones.decay, &zones.tension, &zones.period,
                         &zones.stereoOffset, &zones.mix, &zones.outputGain, &zones.synced,
                         &zones.syncPhase, &zones.blockTick, &zones.phaseOut })
    {
        if (*zone == nullptr)
        {
            jassertfalse; // Ripple.dsp and this class disagree about the control names
            *zone = &unboundZones[(size_t) spare++];
        }
    }
}

FAUSTFLOAT** RippleEngine::Zones::find (const char* label)
{
    const juce::String l (label);
    if (l == "depth")        return &depth;
    if (l == "ripples")      return &ripples;
    if (l == "decay")        return &decay;
    if (l == "tension")      return &tension;
    if (l == "period")       return &period;
    if (l == "stereoOffset") return &stereoOffset;
    if (l == "mix")          return &mix;
    if (l == "outputGain")   return &outputGain;
    if (l == "synced")       return &synced;
    if (l == "syncPhase")    return &syncPhase;
    if (l == "blockTick")    return &blockTick;
    if (l == "phaseOut")     return &phaseOut;
    return nullptr;
}

void RippleEngine::Zones::bind (const char* label, FAUSTFLOAT* zone)
{
    if (auto** slot = find (label))
        *slot = zone;
    else
        jassertfalse; // a control in Ripple.dsp that this class doesn't know about
}

void RippleEngine::prepare (double sampleRate, int maxBlockSize)
{
    // init() also zeroes the clock and the stereo-offset delay line.
    faust.init ((int) std::lround (sampleRate));
    blockTick = false;
    phaseSeconds = 0.0;
    spareChannel.setSize (1, juce::jmax (1, maxBlockSize));
}

void RippleEngine::process (juce::AudioBuffer<float>& buffer, const RippleEngineSettings& s)
{
    *zones.depth        = s.depth;
    *zones.ripples      = (FAUSTFLOAT) s.ripples;
    *zones.decay        = s.decay;
    *zones.tension      = s.tension;
    *zones.period       = (FAUSTFLOAT) s.periodSeconds;
    *zones.stereoOffset = s.stereoOffsetMs;
    *zones.mix          = s.mix;
    *zones.outputGain   = s.outputGainDb;
    *zones.synced       = s.resync ? 1.0f : 0.0f;
    *zones.syncPhase    = (FAUSTFLOAT) s.syncPhaseSeconds;

    // Ripple.dsp re-locks the clock on the first sample after this flips.
    blockTick = ! blockTick;
    *zones.blockTick = blockTick ? 1.0f : 0.0f;

    const int numSamples = buffer.getNumSamples();

    if (buffer.getNumChannels() >= 2)
    {
        float* channels[] = { buffer.getWritePointer (0), buffer.getWritePointer (1) };
        faust.compute (numSamples, channels, channels);
    }
    else if (buffer.getNumChannels() == 1)
    {
        // Chunked in case the host sends a bigger block than prepareToPlay promised. Only the
        // first chunk sees the blockTick flip, so the clock still re-locks once per block.
        const int chunkSize = spareChannel.getNumSamples();
        for (int start = 0; start < numSamples; start += chunkSize)
        {
            const int n = juce::jmin (chunkSize, numSamples - start);
            spareChannel.clear();
            float* channels[] = { buffer.getWritePointer (0) + start, spareChannel.getWritePointer (0) };
            faust.compute (n, channels, channels);
        }
    }

    phaseSeconds = (double) *zones.phaseOut;
}

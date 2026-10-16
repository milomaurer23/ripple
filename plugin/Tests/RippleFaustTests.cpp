// Holds the Faust engine (RippleEngine + Faust/Ripple.dsp) to the C++ engine it replaced.
//
// ReferenceRipple below is the per-sample loop from PluginProcessor::processBlock as it was before
// the port (commit 8630873), driven by RippleGenerator. Both engines get the same noise input and
// the same parameter changes, block by block, across sample rates, block sizes, mono/stereo,
// free-running and tempo-synced (including playhead jumps), and every output sample is compared.
//
// Faust's controls are float, so the period and sync phase reach the Faust clock rounded to float
// (under 0.1 ppm). The reference gets the same rounded values, so this compares the engines and not
// that rounding: an unrounded reference would wrap one sample apart now and then and flag a click
// that the rounding, not the port, put there.

#include <JuceHeader.h>
#include "../Source/RippleEngine.h"
#include "../Source/RippleGenerator.h"
#include "../Source/Parameters.h"

namespace
{
struct ReferenceRipple
{
    RippleGenerator leftGenerator;
    std::vector<float> gainHistory;
    int gainHistoryWrite = 0;
    double currentSampleRate = 44100.0;

    void prepare (double sampleRate)
    {
        currentSampleRate = sampleRate;
        leftGenerator.prepare (sampleRate);
        const int historySize = (int) (0.05 * sampleRate) + 8;
        gainHistory.assign ((size_t) historySize, 1.0f);
        gainHistoryWrite = 0;
    }

    void process (juce::AudioBuffer<float>& buffer, const RippleEngineSettings& s)
    {
        RippleParams p;
        p.depth = s.depth;
        p.ripples = s.ripples;
        p.decay = s.decay;
        p.tension = s.tension;
        p.periodSeconds = s.periodSeconds;
        if (s.resync)
            leftGenerator.setPhaseSeconds (s.syncPhaseSeconds);
        leftGenerator.setParameters (p);

        const float mix = s.mix;
        const float outputGainLinear = juce::Decibels::decibelsToGain (s.outputGainDb);
        const int historySize = (int) gainHistory.size();
        const int delaySamples = historySize > 1
            ? juce::jlimit (0, historySize - 1, (int) ((double) s.stereoOffsetMs / 1000.0 * currentSampleRate))
            : 0;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float gL = leftGenerator.getNextGain();

            float gR = gL;
            if (historySize > 0)
            {
                gainHistory[(size_t) gainHistoryWrite] = gL;
                const int readIndex = ((gainHistoryWrite - delaySamples) % historySize + historySize) % historySize;
                gR = gainHistory[(size_t) readIndex];
                gainHistoryWrite = (gainHistoryWrite + 1) % historySize;
            }

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                const float g = (ch == 1) ? gR : gL;
                auto* data = buffer.getWritePointer (ch);
                const float dry = data[i];
                const float wet = dry * g;
                data[i] = (dry * (1.0f - mix) + wet * mix) * outputGainLinear;
            }
        }
    }
};

double toFloat (double x) { return (double) (float) x; }

// What the plugin's knobs can produce, with the same rounding the Faust controls apply.
RippleEngineSettings randomSettings (juce::Random& rng, bool synced, double bpm, int divIndex)
{
    RippleEngineSettings s;
    s.depth = rng.nextFloat();
    s.ripples = 1 + rng.nextInt (8);
    s.decay = rng.nextFloat();
    s.tension = rng.nextFloat();
    s.stereoOffsetMs = rng.nextFloat() * 50.0f;
    s.mix = rng.nextFloat();
    s.outputGainDb = -12.0f + 24.0f * rng.nextFloat();

    if (synced)
        s.periodSeconds = toFloat (syncDivisions()[(size_t) divIndex].beats * 60.0 / bpm);
    else
    {
        const float rateHz = juce::NormalisableRange<float> (0.05f, 20.0f, 0.0f, 0.35f).convertFrom0to1 (rng.nextFloat());
        s.periodSeconds = toFloat (1.0 / (double) rateHz);
    }
    return s;
}

struct TrialResult { double maxError = 0.0; long long samples = 0; };

TrialResult runTrial (double sampleRate, int numChannels, bool synced, juce::int64 seed)
{
    juce::Random rng (seed);
    constexpr int preparedBlock = 512;

    RippleEngine faust;
    ReferenceRipple reference;
    faust.prepare (sampleRate, preparedBlock);
    reference.prepare (sampleRate);

    const double bpm = 60.0 + 120.0 * rng.nextDouble();
    int divIndex = rng.nextInt ((int) syncDivisions().size());
    double ppq = rng.nextDouble() * 8.0;
    auto settings = randomSettings (rng, synced, bpm, divIndex);

    TrialResult result;
    const int totalSamples = (int) (3.0 * sampleRate);
    int done = 0;

    while (done < totalSamples)
    {
        // Mostly ordinary block sizes, sometimes single samples, sometimes bigger than prepared
        // (mono then exercises RippleEngine's chunking).
        const int pick = rng.nextInt (10);
        const int blockSize = pick == 0 ? 1 : pick == 1 ? 513 + rng.nextInt (1536) : 16 + rng.nextInt (preparedBlock - 15);

        if (rng.nextInt (8) == 0)
        {
            divIndex = rng.nextInt ((int) syncDivisions().size());
            settings = randomSettings (rng, synced, bpm, divIndex);
        }
        if (synced && rng.nextInt (40) == 0)
            ppq = rng.nextDouble() * 16.0; // loop point / locate

        auto blockSettings = settings;
        if (synced)
        {
            const double beats = syncDivisions()[(size_t) divIndex].beats;
            const double phaseBeats = std::fmod (ppq, beats);
            blockSettings.resync = true;
            blockSettings.syncPhaseSeconds = toFloat ((phaseBeats < 0.0 ? phaseBeats + beats : phaseBeats) * 60.0 / bpm);
        }

        juce::AudioBuffer<float> a (numChannels, blockSize);
        for (int ch = 0; ch < numChannels; ++ch)
            for (int i = 0; i < blockSize; ++i)
                a.setSample (ch, i, rng.nextFloat() * 2.0f - 1.0f);
        juce::AudioBuffer<float> b (a);

        faust.process (a, blockSettings);
        reference.process (b, blockSettings);

        for (int ch = 0; ch < numChannels; ++ch)
            for (int i = 0; i < blockSize; ++i)
            {
                const double expected = b.getSample (ch, i);
                const double err = std::abs ((double) a.getSample (ch, i) - expected) / (1.0 + std::abs (expected));
                result.maxError = juce::jmax (result.maxError, err);
            }

        result.samples += blockSize;
        done += blockSize;
        ppq += blockSize * bpm / 60.0 / sampleRate;
    }
    return result;
}
} // namespace

int main()
{
    constexpr double tolerance = 1.0e-5; // relative; float output rounding is ~1e-7
    bool ok = true;
    juce::int64 seed = 1;

    for (double sr : { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 })
        for (int channels : { 2, 1 })
            for (bool synced : { false, true })
            {
                TrialResult worst;
                for (int trial = 0; trial < 6; ++trial)
                {
                    const auto r = runTrial (sr, channels, synced, seed++);
                    worst.maxError = juce::jmax (worst.maxError, r.maxError);
                    worst.samples += r.samples;
                }

                const bool pass = worst.maxError < tolerance;
                ok = ok && pass;
                std::printf ("%s  %6.0f Hz  %s  %-5s  max rel error %.2e over %lld samples\n",
                             pass ? "PASS" : "FAIL", sr, channels == 2 ? "stereo" : "mono  ",
                             synced ? "sync" : "free", worst.maxError, worst.samples);
            }

    std::printf (ok ? "\nFaust engine matches the C++ engine.\n" : "\nMISMATCH between the Faust and C++ engines.\n");
    return ok ? 0 : 1;
}

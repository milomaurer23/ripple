#include "RippleGenerator.h"

void RippleGenerator::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
}

float RippleGenerator::computeGain (const RippleParams& params, double time)
{
    if (params.periodSeconds <= 0.0)
        return 1.0f;

    const double tau = params.periodSeconds * (0.1 + 0.9 * (double) params.decay);
    const double ringHz = (double) params.ripples / params.periodSeconds;

    const double h = std::exp (-time / tau)
                    * std::cos (2.0 * juce::MathConstants<double>::pi * ringHz * time);
    const double shaped = (1.0 - (double) params.tension) * h
                         + (double) params.tension * -std::abs (h);

    // Short raised-cosine ramp-in on every retrigger, since the previous cycle's tail may not
    // have fully decayed to unity by the time the next one starts.
    constexpr double attackSeconds = 0.0015;
    double attackGate = 1.0;
    if (time < attackSeconds)
        attackGate = 0.5 - 0.5 * std::cos (juce::MathConstants<double>::pi * (time / attackSeconds));

    const double gain = 1.0 + (double) params.depth * shaped * attackGate;
    return (float) juce::jlimit (0.0, 2.0, gain);
}

float RippleGenerator::getNextGain()
{
    const float gain = computeGain (params, t);

    t += 1.0 / sampleRate;
    if (params.periodSeconds > 0.0 && t >= params.periodSeconds)
        t = std::fmod (t, params.periodSeconds);

    return gain;
}

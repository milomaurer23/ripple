#pragma once
#include <JuceHeader.h>

// One damped-harmonic-oscillator "droplet": each trigger period, a decaying cosine rings out
// and settles back to unity gain. Tension morphs it from a symmetric ripple (above/below unity)
// toward a one-directional duck. See RippleGenerator.cpp for the formula.
struct RippleParams
{
    float depth = 0.7f;          // 0..1, contribution of the ripple to the gain
    int ripples = 3;             // how many oscillations complete within one period
    float decay = 0.5f;          // 0..1, maps to the decay time constant
    float tension = 0.3f;        // 0..1, ripple <-> one-directional duck
    double periodSeconds = 0.5;  // trigger period; 0 disables retriggering (gain stays at unity)
};

class RippleGenerator
{
public:
    void prepare (double newSampleRate);
    void setParameters (const RippleParams& newParams) { params = newParams; }
    void reset() { t = 0.0; }

    // Forces the internal phase, in seconds since the last trigger. Used when tempo-synced to
    // lock to the host's ppqPosition every block instead of free-running (which would drift).
    void setPhaseSeconds (double phaseSeconds) { t = phaseSeconds; }
    double getPhaseSeconds() const { return t; }

    float getNextGain();

    // Pure function of the current parameters, used by both getNextGain() and the GUI curve
    // display so what's drawn is exactly what's applied.
    static float computeGain (const RippleParams& params, double time);

private:
    double sampleRate = 44100.0;
    double t = 0.0;
    RippleParams params;
};

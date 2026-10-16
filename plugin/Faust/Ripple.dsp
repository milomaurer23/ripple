// Ripple's audio path in Faust: the droplet clock, the damped-oscillator gain, the stereo
// offset, Mix and Output. A port of RippleGenerator + the old per-sample loop in
// PluginProcessor::processBlock, kept sample-for-sample equivalent (RippleFaustTests checks it).
//
// The host glue stays in C++: PluginProcessor reads the playhead and turns Rate / Sync Division
// into `period`, and when tempo-synced it passes the playhead phase in `syncPhase` and flips
// `blockTick` once per block so the clock re-locks at the first sample of every block.
//
// Regenerate the C++ class with scripts/faust.sh after editing this file.

declare name "Ripple";
declare author "Milo Maurer";

import("stdfaust.lib");

depth      = hslider("depth", 0.7, 0, 1, 0.001);
ripples    = hslider("ripples", 3, 1, 8, 1);
decay      = hslider("decay", 0.5, 0, 1, 0.001);
tension    = hslider("tension", 0.3, 0, 1, 0.001);
period     = hslider("period [unit:s]", 0.5, 0, 10000, 0.0001);   // trigger period; 0 = unity gain
stereoMs   = hslider("stereoOffset [unit:ms]", 8, 0, 50, 0.01);
mix        = hslider("mix", 1, 0, 1, 0.001);
outputDb   = hslider("outputGain [unit:dB]", 0, -12, 12, 0.01);
synced     = checkbox("synced");
syncPhase  = hslider("syncPhase [unit:s]", 0, 0, 10000, 0.0001);
blockTick  = checkbox("blockTick");

// Seconds since the last trigger. `next` holds the time for the following sample; the time used
// now is next' unless a new block arrived while synced, in which case it snaps to syncPhase.
phase = next' : relock
with {
    relock(t) = select2(synced & (blockTick != blockTick'), t, syncPhase);
    wrap(t)   = select2((period > 0) & (t >= period), t, fmod(t, period));
    next      = (relock : +(1.0 / ma.SR) : wrap) ~ _;
};

// Same formula as RippleGenerator::computeGain, which the editor's curve view still draws from.
rippleGain(t) = select2(period > 0, 1.0, gain)
with {
    tau      = period * (0.1 + 0.9 * decay);
    ringHz   = ripples / max(period, ma.EPSILON);
    h        = exp(0 - t / tau) * cos(2.0 * ma.PI * ringHz * t);
    shaped   = (1.0 - tension) * h + tension * (0 - abs(h));
    attackS  = 0.0015;
    gate     = select2(t < attackS, 1.0, 0.5 - 0.5 * cos(ma.PI * (t / attackS)));
    gain     = max(0.0, min(2.0, 1.0 + depth * shaped * gate));
};

// The right channel's gain is the left's, delayed. Delaying (g - 1) keeps the right channel at
// unity before the delay line fills, as the old history buffer did.
maxDelay      = 16384;   // 50 ms at 192 kHz is 9600 samples
offsetSamples = int(stereoMs / 1000.0 * ma.SR) : min(maxDelay - 1) : max(0);
delayedGain(g) = 1.0 + de.delay(maxDelay, offsetSamples, g - 1.0);

outLin = ba.db2linear(outputDb);
apply(g, x) = (x * (1.0 - mix) + x * g * mix) * outLin;

gainL = phase : attach(rippleGain, phase : hbargraph("phaseOut [unit:s]", 0, 10000));
process(l, r) = gainL <: (_, delayedGain) : (apply(_, l), apply(_, r));

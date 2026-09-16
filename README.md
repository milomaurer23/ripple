# Ripple

An organic modulation source for VST3/AU hosts — instead of drawing a curve or dialing in an LFO shape, Ripple models a droplet hitting water: each retrigger rings out as a damped harmonic oscillator and settles back to unity gain.

## Parameters

- **Depth** — contribution of the ripple to the output gain
- **Ripples** — how many oscillations complete within one period
- **Decay** — maps to the ripple's decay time constant
- **Tension** — morphs the ripple from symmetric (above/below unity) toward a one-directional duck

## Status

v1 scaffold. Compiles clean under a strict warning set; both VST3 and AU pass `pluginval --strictness-level 5` (audio processing, state, automation, auval, VST3 validator, bus layouts). Not yet evaluated by ear in a real DAW session.

## Build

```
cmake -B build
cmake --build build --config Release --target Ripple_VST3 Ripple_AU
```

Or run the full verification pass:

```
./scripts/pluginval.sh
```

Built with [JUCE](https://juce.com).

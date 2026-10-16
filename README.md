# Ripple

An organic modulation source for VST3/AU hosts — instead of drawing a curve or dialing in an LFO shape, Ripple models a droplet hitting water: each retrigger rings out as a damped harmonic oscillator and settles back to unity gain.

## Parameters

- **Depth** — contribution of the ripple to the output gain
- **Ripples** — how many oscillations complete within one period
- **Decay** — maps to the ripple's decay time constant
- **Tension** — morphs the ripple from symmetric (above/below unity) toward a one-directional duck

## DSP in Faust

The audio path is written in [Faust](https://faust.grame.fr): `plugin/Faust/Ripple.dsp` holds the
droplet clock, the ripple gain, the stereo offset, Mix and Output. `scripts/faust.sh` compiles it
to `plugin/Source/RippleDSP.h`, which is committed, so building the plugin does not need Faust.
Only editing the `.dsp` does (`brew install faust`, then `scripts/faust.sh`).

JUCE still does everything around it: parameters, the playhead and tempo sync, the editor.
`RippleEngine` feeds the Faust controls once per block. `RippleGenerator` stays as the source of
the editor's curve.

`plugin/Tests/RippleFaustTests.cpp` runs the Faust engine and the C++ engine it replaced side by
side and compares every output sample. It covers 44.1 to 192 kHz, mono and stereo, free-running
and tempo-synced with playhead jumps, and random parameter changes and block sizes. The largest
difference is 2.2e-7, which is float rounding.

To hear the DSP without building anything, paste `Ripple.dsp` into the
[Faust IDE](https://faustide.grame.fr). `period` stands in for Rate and Sync.

## Status

v1 scaffold. Compiles clean under a strict warning set; both VST3 and AU pass `pluginval --strictness-level 5` (audio processing, state, automation, auval, VST3 validator, bus layouts). Not yet evaluated by ear in a real DAW session.

Since the Faust port, pluginval strictness 5 has passed on a Linux VST3 build (JUCE 9.0.4). The
macOS VST3 and AU have not been re-run since the port. `scripts/pluginval.sh` does that.

## Build

```
cmake -B build
cmake --build build --config Release --target Ripple_VST3 Ripple_AU
```

Or run the full verification pass (Faust header check, engine equivalence test, pluginval):

```
./scripts/pluginval.sh
```

Built with [JUCE](https://juce.com).

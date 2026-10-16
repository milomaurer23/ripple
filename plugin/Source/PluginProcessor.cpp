#include "PluginProcessor.h"
#include "PluginEditor.h"

RippleAudioProcessor::RippleAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    , state (*this, nullptr, "RIPPLE", createRippleParameterLayout())
{
    pRateHz       = state.getRawParameterValue (ParamIDs::rateHz);
    pSyncDiv      = state.getRawParameterValue (ParamIDs::syncDiv);
    pSyncMode     = state.getRawParameterValue (ParamIDs::syncMode);
    pDepth        = state.getRawParameterValue (ParamIDs::depth);
    pRipples      = state.getRawParameterValue (ParamIDs::ripples);
    pDecay        = state.getRawParameterValue (ParamIDs::decay);
    pTension      = state.getRawParameterValue (ParamIDs::tension);
    pStereoOffset = state.getRawParameterValue (ParamIDs::stereoOffset);
    pMix          = state.getRawParameterValue (ParamIDs::mix);
    pOutputGain   = state.getRawParameterValue (ParamIDs::outputGain);
}

void RippleAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
}

void RippleAudioProcessor::releaseResources() {}

bool RippleAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto set = layouts.getMainOutputChannelSet();
    if (set != juce::AudioChannelSet::mono() && set != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == set;
}

RippleEngineSettings RippleAudioProcessor::buildEngineSettings (double hostBpm, double ppqPosition)
{
    RippleEngineSettings s;
    s.depth = pDepth->load();
    s.ripples = (int) pRipples->load();
    s.decay = pDecay->load();
    s.tension = pTension->load();
    s.stereoOffsetMs = pStereoOffset->load();
    s.mix = pMix->load();
    s.outputGainDb = pOutputGain->load();

    const bool synced = pSyncMode->load() > 0.5f;
    const auto& divs = syncDivisions();
    const int divIndex = juce::jlimit (0, (int) divs.size() - 1, (int) pSyncDiv->load());

    if (synced && hostBpm > 0.0)
    {
        // Lock to the host's ppqPosition every block instead of free-running (which would drift).
        const double beats = divs[(size_t) divIndex].beats;
        s.periodSeconds = beats * 60.0 / hostBpm;

        const double phaseBeats = std::fmod (ppqPosition, beats);
        s.resync = true;
        s.syncPhaseSeconds = (phaseBeats < 0.0 ? phaseBeats + beats : phaseBeats) * 60.0 / hostBpm;
    }
    else
    {
        const double rate = juce::jmax (0.0001, (double) pRateHz->load());
        s.periodSeconds = 1.0 / rate;
    }

    return s;
}

void RippleAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    double hostBpm = 120.0;
    double ppqPosition = 0.0;

    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            hostBpm = position->getBpm().orFallback (120.0);
            ppqPosition = position->getPpqPosition().orFallback (0.0);
        }
    }

    const auto settings = buildEngineSettings (hostBpm, ppqPosition);

    RippleParams curve;
    curve.depth = settings.depth;
    curve.ripples = settings.ripples;
    curve.decay = settings.decay;
    curve.tension = settings.tension;
    curve.periodSeconds = settings.periodSeconds;
    lastRippleParams = curve;

    engine.process (buffer, settings);
}

juce::AudioProcessorEditor* RippleAudioProcessor::createEditor()
{
    return new RippleAudioProcessorEditor (*this);
}

void RippleAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = state.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void RippleAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        state.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RippleAudioProcessor();
}

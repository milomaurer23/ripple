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

void RippleAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    leftGenerator.prepare (sampleRate);

    const int historySize = (int) (0.05 * sampleRate) + 8; // covers max 50ms stereo offset
    gainHistory.assign ((size_t) historySize, 1.0f);
    gainHistoryWrite = 0;
}

void RippleAudioProcessor::releaseResources() {}

bool RippleAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto set = layouts.getMainOutputChannelSet();
    if (set != juce::AudioChannelSet::mono() && set != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == set;
}

RippleParams RippleAudioProcessor::buildRippleParams (double sampleRate, double hostBpm, double ppqPosition, bool isPlaying)
{
    juce::ignoreUnused (sampleRate, isPlaying);

    RippleParams p;
    p.depth = pDepth->load();
    p.ripples = (int) pRipples->load();
    p.decay = pDecay->load();
    p.tension = pTension->load();

    const bool synced = pSyncMode->load() > 0.5f;
    const auto& divs = syncDivisions();
    const int divIndex = juce::jlimit (0, (int) divs.size() - 1, (int) pSyncDiv->load());

    if (synced && hostBpm > 0.0)
    {
        const double beats = divs[(size_t) divIndex].beats;
        p.periodSeconds = beats * 60.0 / hostBpm;

        const double phaseBeats = std::fmod (ppqPosition, beats);
        const double phaseSeconds = (phaseBeats < 0.0 ? phaseBeats + beats : phaseBeats) * 60.0 / hostBpm;
        leftGenerator.setPhaseSeconds (phaseSeconds);
    }
    else
    {
        const double rate = juce::jmax (0.0001, (double) pRateHz->load());
        p.periodSeconds = 1.0 / rate;
    }

    return p;
}

void RippleAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    double hostBpm = 120.0;
    double ppqPosition = 0.0;
    bool isPlaying = false;

    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            hostBpm = position->getBpm().orFallback (120.0);
            ppqPosition = position->getPpqPosition().orFallback (0.0);
            isPlaying = position->getIsPlaying();
        }
    }

    const auto rippleParams = buildRippleParams (currentSampleRate, hostBpm, ppqPosition, isPlaying);
    leftGenerator.setParameters (rippleParams);
    lastRippleParams = rippleParams;

    const float mix = pMix->load();
    const float outputGainLinear = juce::Decibels::decibelsToGain (pOutputGain->load());
    const int historySize = (int) gainHistory.size();
    const int delaySamples = historySize > 1
        ? juce::jlimit (0, historySize - 1, (int) ((double) pStereoOffset->load() / 1000.0 * currentSampleRate))
        : 0;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
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

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float g = (ch == 1) ? gR : gL;
            auto* data = buffer.getWritePointer (ch);
            const float dry = data[i];
            const float wet = dry * g;
            data[i] = (dry * (1.0f - mix) + wet * mix) * outputGainLinear;
        }
    }
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

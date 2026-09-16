#include "RippleView.h"
#include "PluginProcessor.h"

RippleView::RippleView (RippleAudioProcessor& processorIn) : processor (processorIn)
{
    startTimerHz (30);
}

void RippleView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colour::fromRGB (0x14, 0x1b, 0x22));
    g.fillRoundedRectangle (bounds, 6.0f);

    const auto params = processor.getCurrentRippleParams();
    if (params.periodSeconds <= 0.0)
        return;

    const float midY = bounds.getCentreY();
    const float halfHeight = bounds.getHeight() * 0.42f;

    juce::Path curve;
    const int numPoints = juce::jmax (2, (int) bounds.getWidth());
    for (int i = 0; i < numPoints; ++i)
    {
        const double time = params.periodSeconds * (double) i / (double) (numPoints - 1);
        const float gain = RippleGenerator::computeGain (params, time);
        const float x = bounds.getX() + bounds.getWidth() * (float) i / (float) (numPoints - 1);
        const float y = midY - (gain - 1.0f) * halfHeight;
        if (i == 0) curve.startNewSubPath (x, y);
        else curve.lineTo (x, y);
    }

    g.setColour (juce::Colours::white.withAlpha (0.15f));
    g.drawHorizontalLine ((int) midY, bounds.getX(), bounds.getRight());

    g.setColour (juce::Colour::fromRGB (0x6f, 0xc7, 0xe8));
    g.strokePath (curve, juce::PathStrokeType (2.0f));

    const double phase = std::fmod (processor.getGeneratorPhaseSeconds(), params.periodSeconds);
    const float markerX = bounds.getX() + bounds.getWidth() * (float) (phase / params.periodSeconds);
    g.setColour (juce::Colours::white.withAlpha (0.6f));
    g.drawVerticalLine ((int) markerX, bounds.getY(), bounds.getBottom());
}

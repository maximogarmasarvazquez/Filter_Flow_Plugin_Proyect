#include "PluginEditor.h"
#include "PluginProcessor.h"

Filter_FlowAudioProcessorEditor::Filter_FlowAudioProcessorEditor(Filter_FlowAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Inicializamos attachments con make_unique
    lowGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "lowGain", lowGainSlider);
    midGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "midGain", midGainSlider);
    highGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "highGain", highGainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "gain", gainSlider);
    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "pan", panSlider);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "bypass", bypassButton);
  
    setSize(600, 400);
    getLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::skyblue);

    // Setup sliders
    auto setupSlider = [](juce::Slider& slider, juce::Label& label, const juce::String& labelText)
        {
            slider.setSliderStyle(juce::Slider::Rotary);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
            label.setText(labelText, juce::dontSendNotification);
            label.attachToComponent(&slider, false);
        };

    setupSlider(lowGainSlider, lowGainLabel, "Low");
    setupSlider(midGainSlider, midGainLabel, "Mid");
    setupSlider(highGainSlider, highGainLabel, "High");
    setupSlider(gainSlider, gainLabel, "Gain");
    setupSlider(panSlider, panLabel, "Pan");
    bypassButton.setButtonText("Bypass");

    addAndMakeVisible(bypassButton);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(panSlider);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(panLabel);

    addAndMakeVisible(lowGainSlider);
    addAndMakeVisible(midGainSlider);
    addAndMakeVisible(highGainSlider);
    addAndMakeVisible(lowGainLabel);
    addAndMakeVisible(midGainLabel);
    addAndMakeVisible(highGainLabel);

    startTimerHz(30); // Redibuja el visualizador 30 veces por segundo
}

Filter_FlowAudioProcessorEditor::~Filter_FlowAudioProcessorEditor() {}

float Filter_FlowAudioProcessorEditor::calculateMagnitudeForFrequency(const juce::dsp::IIR::Filter<float>& filter, float sampleRate, float frequency) const
{
    auto coeffs = filter.coefficients;
    if (coeffs == nullptr)
        return 0.0f;

    double w = 2.0 * juce::MathConstants<double>::pi * frequency / sampleRate;

    const auto& c = *coeffs;

    std::complex<double> numerator = std::complex<double>(
        c.coefficients[0] + c.coefficients[1] * std::cos(-w) + c.coefficients[2] * std::cos(-2 * w),
        c.coefficients[1] * std::sin(-w) + c.coefficients[2] * std::sin(-2 * w)
    );

    std::complex<double> denominator = std::complex<double>(
        1.0 + c.coefficients[3] * std::cos(-w) + c.coefficients[4] * std::cos(-2 * w),
        c.coefficients[3] * std::sin(-w) + c.coefficients[4] * std::sin(-2 * w)
    );

    std::complex<double> H = numerator / denominator;

    return static_cast<float>(std::abs(H));
}

void Filter_FlowAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 30)); // gris oscuro

    g.setColour(juce::Colours::green);
    g.strokePath(lowBandPath, juce::PathStrokeType(2.0f));

    g.setColour(juce::Colours::yellow);
    g.strokePath(midBandPath, juce::PathStrokeType(2.0f));

    g.setColour(juce::Colours::red);
    g.strokePath(highBandPath, juce::PathStrokeType(2.0f));
}

void Filter_FlowAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    bypassButton.setBounds(area.removeFromBottom(30).removeFromLeft(100));

    // EQ Visualizer en la parte superior
    auto visualizerArea = area.removeFromTop(150);

    // Primera fila: Gain y Pan
    auto topRow = area.removeFromTop(100);
    auto controlWidth = 100;
    gainSlider.setBounds(topRow.removeFromLeft(controlWidth).reduced(10));
    panSlider.setBounds(topRow.removeFromLeft(controlWidth).reduced(10));

    // Segunda fila: Low, Mid, High
    auto eqRow = area.removeFromTop(150);
    lowGainSlider.setBounds(eqRow.removeFromLeft(controlWidth).reduced(10));
    midGainSlider.setBounds(eqRow.removeFromLeft(controlWidth).reduced(10));
    highGainSlider.setBounds(eqRow.removeFromLeft(controlWidth).reduced(10));
}

void Filter_FlowAudioProcessorEditor::timerCallback()
{
    updateEQVisualization();
    repaint();
}

void Filter_FlowAudioProcessorEditor::updateEQVisualization()
{
    auto bounds = getLocalBounds().reduced(20).removeFromTop(200);
    auto width = bounds.getWidth();

    lowBandPath.clear();
    midBandPath.clear();
    highBandPath.clear();

    const int numPoints = width;
    float sampleRate = audioProcessor.getSampleRate();

    for (int i = 0; i < numPoints; ++i)
    {
        float freq = 20.0f * static_cast<float>(std::pow(10.0f, 3.0f * i / (numPoints - 1)));

        float magLow = calculateMagnitudeForFrequency(audioProcessor.eq.getLowFilter(), sampleRate, freq);
        float magMid = calculateMagnitudeForFrequency(audioProcessor.eq.getMidFilter(), sampleRate, freq);
        float magHigh = calculateMagnitudeForFrequency(audioProcessor.eq.getHighFilter(), sampleRate, freq);

        auto mapToY = [&](float mag)
            {
                float dB = juce::Decibels::gainToDecibels(mag);
                return juce::jmap(dB, -24.0f, 24.0f, (float)bounds.getBottom(), (float)bounds.getY());
            };

        float x = bounds.getX() + (float)i;

        if (i == 0)
        {
            lowBandPath.startNewSubPath(x, mapToY(magLow));
            midBandPath.startNewSubPath(x, mapToY(magMid));
            highBandPath.startNewSubPath(x, mapToY(magHigh));
        }
        else
        {
            lowBandPath.lineTo(x, mapToY(magLow));
            midBandPath.lineTo(x, mapToY(magMid));
            highBandPath.lineTo(x, mapToY(magHigh));
        }
    }
}

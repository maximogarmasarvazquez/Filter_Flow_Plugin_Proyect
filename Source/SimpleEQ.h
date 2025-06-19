#pragma once

#include <JuceHeader.h>

class SimpleEQ
{
public:
    SimpleEQ();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void setGains(float low, float mid, float high);
    void process(juce::AudioBuffer<float>& buffer);

    const juce::dsp::IIR::Filter<float>& getLowFilter() const { return lowFilter; }
    const juce::dsp::IIR::Filter<float>& getMidFilter() const { return midFilter; }
    const juce::dsp::IIR::Filter<float>& getHighFilter() const { return highFilter; }

private:
    double sampleRate = 44100.0;

    juce::dsp::IIR::Filter<float> lowFilter, midFilter, highFilter;
    juce::dsp::IIR::Coefficients<float>::Ptr lowCoefficients, midCoefficients, highCoefficients;
};

#include "SimpleEQ.h"
#include "SimpleEQ.h"

SimpleEQ::SimpleEQ()
{
    lowCoefficients = nullptr;
    midCoefficients = nullptr;
    highCoefficients = nullptr;
}

void SimpleEQ::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    lowCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 100.0f, 0.707f, juce::Decibels::decibelsToGain(0.0f));
    midCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 0.707f, juce::Decibels::decibelsToGain(0.0f));
    highCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 5000.0f, 0.707f, juce::Decibels::decibelsToGain(0.0f));

    lowFilter.coefficients = lowCoefficients;
    midFilter.coefficients = midCoefficients;
    highFilter.coefficients = highCoefficients;

    lowFilter.prepare(spec);
    midFilter.prepare(spec);
    highFilter.prepare(spec);
}

void SimpleEQ::setGains(float low, float mid, float high)
{
    *lowCoefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 100.0f, 0.707f, juce::Decibels::decibelsToGain(low));
    *midCoefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 0.707f, juce::Decibels::decibelsToGain(mid));
    *highCoefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 5000.0f, 0.707f, juce::Decibels::decibelsToGain(high));
}

void SimpleEQ::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    lowFilter.process(context);
    midFilter.process(context);
    highFilter.process(context);
}

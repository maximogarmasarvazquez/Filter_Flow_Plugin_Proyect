#pragma once

#include <JuceHeader.h>
#include "SimpleEQ.h"

class Filter_FlowAudioProcessor : public juce::AudioProcessor
{
public:
    Filter_FlowAudioProcessor();
    ~Filter_FlowAudioProcessor() override;

    // Métodos esenciales del plugin
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // Información del plugin
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // Programas (presets)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    // Estado
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Buses layout (importante para standalone y DAWs)
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    // Parámetros del plugin
    juce::AudioProcessorValueTreeState parameters;

    // Procesador de ecualización
    SimpleEQ eq;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Filter_FlowAudioProcessor)
};

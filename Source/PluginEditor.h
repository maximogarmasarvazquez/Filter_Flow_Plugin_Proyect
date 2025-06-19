#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class Filter_FlowAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    Filter_FlowAudioProcessorEditor(Filter_FlowAudioProcessor&);
    ~Filter_FlowAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Función para calcular la magnitud en frecuencia de un filtro
    float calculateMagnitudeForFrequency(const juce::dsp::IIR::Filter<float>& filter, float sampleRate, float frequency) const;

    void timerCallback() override; // Para actualizar la visualización

    void updateEQVisualization();

    Filter_FlowAudioProcessor& audioProcessor;

    // Sliders y Labels para Gain y Pan
    juce::Slider gainSlider;
    juce::Slider panSlider;
    juce::Label gainLabel;
    juce::Label panLabel;

    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    // Attachments como punteros inteligentes para evitar problemas
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;

    // Sliders y Labels para Low, Mid, High Gain
    juce::Slider lowGainSlider;
    juce::Slider midGainSlider;
    juce::Slider highGainSlider;

    juce::Label lowGainLabel;
    juce::Label midGainLabel;
    juce::Label highGainLabel;

    // Attachments para Low, Mid, High como punteros inteligentes también
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highGainAttachment;

    // Para dibujar las curvas de respuesta EQ
    juce::Path lowBandPath, midBandPath, highBandPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Filter_FlowAudioProcessorEditor)
};

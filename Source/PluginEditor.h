#pragma once  // Previene que este archivo se incluya más de una vez

#include <JuceHeader.h>  // Incluye la librería JUCE principal
#include "PluginProcessor.h"  // Incluye la definición del procesador de audio

// Clase que maneja la interfaz gráfica (GUI) del plugin
// Hereda de AudioProcessorEditor para poder mostrar controles y gráficos
// También hereda de Timer para actualizar visualizaciones periódicamente
class Filter_FlowAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    // Constructor: recibe referencia al procesador de audio para interactuar con él
    Filter_FlowAudioProcessorEditor(Filter_FlowAudioProcessor&);

    // Destructor
    ~Filter_FlowAudioProcessorEditor() override;

    // Método para pintar (dibujar) la interfaz gráfica cada vez que se necesita refrescar la pantalla
    void paint(juce::Graphics&) override;

    // Método llamado cuando la ventana cambia de tamaño, para acomodar los controles
    void resized() override;

private:
    // Función auxiliar para calcular la magnitud (ganancia) de un filtro IIR a una frecuencia dada
    float calculateMagnitudeForFrequency(const juce::dsp::IIR::Filter<float>& filter,
        float sampleRate, float frequency) const;

    // Método que se llama periódicamente por el Timer para actualizar la visualización de la EQ
    void timerCallback() override;

    // Método propio que actualiza las curvas visuales de la EQ en la interfaz
    void updateEQVisualization();

    Filter_FlowAudioProcessor& audioProcessor;  // Referencia al procesador para acceder a sus datos y parámetros

    // Controles (sliders y etiquetas) para controlar Ganancia (Gain) y Pan (balance estéreo)
    juce::Slider gainSlider;
    juce::Slider panSlider;
    juce::Label gainLabel;
    juce::Label panLabel;

    juce::ToggleButton bypassButton;  // Botón para activar/desactivar el plugin
    // Attachment que conecta el botón a un parámetro del AudioProcessorValueTreeState para sincronizar valores
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    // Attachments para sincronizar sliders de ganancia y pan con los parámetros del plugin
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;

    // Controles para las ganancias del ecualizador: Low, Mid y High
    juce::Slider lowGainSlider;
    juce::Slider midGainSlider;
    juce::Slider highGainSlider;

    juce::Label lowGainLabel;
    juce::Label midGainLabel;
    juce::Label highGainLabel;

    // Attachments para sincronizar estos sliders con los parámetros de la EQ en el procesador
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highGainAttachment;

    // Objetos Path para dibujar las curvas de respuesta en la interfaz gráfica de cada banda del EQ
    juce::Path lowBandPath, midBandPath, highBandPath;

    // Macro de JUCE para evitar copiar esta clase y para activar la detección de fugas de memoria (debug)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Filter_FlowAudioProcessorEditor)
};

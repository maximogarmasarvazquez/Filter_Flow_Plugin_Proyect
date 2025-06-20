#pragma once  // Evita incluir este archivo más de una vez durante la compilación

#include <JuceHeader.h>  // Incluye todo JUCE

// Clase SimpleEQ: ecualizador simple con tres bandas (bajo, medio, alto)
class SimpleEQ
{
public:
    SimpleEQ();  // Constructor

    // Prepara el procesador con las especificaciones de audio (sample rate, block size, canales)
    void prepare(const juce::dsp::ProcessSpec& spec);

    // Ajusta las ganancias de las tres bandas (low, mid, high)
    void setGains(float low, float mid, float high);

    // Procesa un buffer de audio aplicando el ecualizador
    void process(juce::AudioBuffer<float>& buffer);

    // Métodos para acceder a los filtros individuales (constantes, solo lectura)

    const juce::dsp::IIR::Filter<float>& getLowFilter() const { return lowFilter; }
    const juce::dsp::IIR::Filter<float>& getMidFilter() const { return midFilter; }
    const juce::dsp::IIR::Filter<float>& getHighFilter() const { return highFilter; }

private:
    double sampleRate = 44100.0; // Frecuencia de muestreo por defecto

    // Filtros IIR para cada banda del ecualizador
    juce::dsp::IIR::Filter<float> lowFilter, midFilter, highFilter;

    // Punteros a las coeficientes de los filtros, que definen sus características (frecuencia, Q, ganancia)
    juce::dsp::IIR::Coefficients<float>::Ptr lowCoefficients, midCoefficients, highCoefficients;
};

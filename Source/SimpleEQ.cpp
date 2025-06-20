#include "SimpleEQ.h"

// Constructor: inicializa los punteros a coeficientes en nullptr
SimpleEQ::SimpleEQ()
{
    lowCoefficients = nullptr;
    midCoefficients = nullptr;
    highCoefficients = nullptr;
}

// Prepara los filtros con las especificaciones del audio (sample rate, block size, canales)
void SimpleEQ::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;  // Guarda la frecuencia de muestreo actual

    // Crea coeficientes para el filtro Low Shelf (bajos) con frecuencia 100 Hz, Q = 0.707, ganancia 0 dB (sin cambio)
    lowCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 100.0f, 0.707f, juce::Decibels::decibelsToGain(0.0f));

    // Crea coeficientes para el filtro Peak (medio) con frecuencia 1000 Hz, Q = 0.707, ganancia 0 dB
    midCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 0.707f, juce::Decibels::decibelsToGain(0.0f));

    // Crea coeficientes para el filtro High Shelf (agudos) con frecuencia 5000 Hz, Q = 0.707, ganancia 0 dB
    highCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 5000.0f, 0.707f, juce::Decibels::decibelsToGain(0.0f));

    // Asigna los coeficientes a cada filtro correspondiente
    lowFilter.coefficients = lowCoefficients;
    midFilter.coefficients = midCoefficients;
    highFilter.coefficients = highCoefficients;

    // Prepara los filtros con las especificaciones (para que se configuren internamente)
    lowFilter.prepare(spec);
    midFilter.prepare(spec);
    highFilter.prepare(spec);
}

// Actualiza las ganancias de los filtros con los valores recibidos (en decibeles)
void SimpleEQ::setGains(float low, float mid, float high)
{
    // Recalcula los coeficientes para el filtro de bajos con la nueva ganancia low (convertida de dB a ganancia lineal)
    *lowCoefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 100.0f, 0.707f, juce::Decibels::decibelsToGain(low));

    // Recalcula los coeficientes para el filtro de medios
    *midCoefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 0.707f, juce::Decibels::decibelsToGain(mid));

    // Recalcula los coeficientes para el filtro de agudos
    *highCoefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 5000.0f, 0.707f, juce::Decibels::decibelsToGain(high));
}

// Procesa el buffer de audio aplicando los tres filtros en serie
void SimpleEQ::process(juce::AudioBuffer<float>& buffer)
{
    // Crea un bloque de audio que envuelve al buffer para procesarlo
    juce::dsp::AudioBlock<float> block(buffer);

    // Crea un contexto de procesamiento que reemplaza el buffer original (in-place processing)
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Procesa el audio con cada filtro (low, mid, high) aplicándolos en orden
    lowFilter.process(context);
    midFilter.process(context);
    highFilter.process(context);
}

#include "PluginEditor.h"
#include "PluginProcessor.h"

// Constructor de la clase editor, recibe referencia al procesador
Filter_FlowAudioProcessorEditor::Filter_FlowAudioProcessorEditor(Filter_FlowAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Creamos los attachments para conectar sliders y botón con los parámetros del plugin
    lowGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "lowGain", lowGainSlider);
    midGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "midGain", midGainSlider);
    highGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "highGain", highGainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "gain", gainSlider);
    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "pan", panSlider);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "bypass", bypassButton);

    setSize(600, 400);  // Tamaño inicial de la ventana del plugin

    // Cambiamos colores de los controles para mejorar la apariencia
    getLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::skyblue);

    // Función lambda para configurar sliders y labels con estilo rotary y textbox debajo
    auto setupSlider = [](juce::Slider& slider, juce::Label& label, const juce::String& labelText)
        {
            slider.setSliderStyle(juce::Slider::Rotary);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
            label.setText(labelText, juce::dontSendNotification);
            label.attachToComponent(&slider, false);
        };

    // Configuramos los sliders y labels para cada parámetro
    setupSlider(lowGainSlider, lowGainLabel, "Low");
    setupSlider(midGainSlider, midGainLabel, "Mid");
    setupSlider(highGainSlider, highGainLabel, "High");
    setupSlider(gainSlider, gainLabel, "Gain");
    setupSlider(panSlider, panLabel, "Pan");

    bypassButton.setButtonText("Bypass");  // Texto para el botón bypass

    // Añadimos controles a la interfaz para que sean visibles y reciban eventos
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

    startTimerHz(30); // Arranca un timer que llama timerCallback 30 veces por segundo para actualizar la interfaz
}

// Destructor vacío (puede usarse para limpieza si se necesita)
Filter_FlowAudioProcessorEditor::~Filter_FlowAudioProcessorEditor() {}

// Función que calcula la magnitud (ganancia) de un filtro IIR a una frecuencia dada
float Filter_FlowAudioProcessorEditor::calculateMagnitudeForFrequency(const juce::dsp::IIR::Filter<float>& filter, float sampleRate, float frequency) const
{
    auto coeffs = filter.coefficients;
    if (coeffs == nullptr)  // Si el filtro no tiene coeficientes, retornamos 0
        return 0.0f;

    // Calculamos la frecuencia angular normalizada
    double w = 2.0 * juce::MathConstants<double>::pi * frequency / sampleRate;

    const auto& c = *coeffs;

    // Numerador de la función de transferencia H(z)
    std::complex<double> numerator = std::complex<double>(
        c.coefficients[0] + c.coefficients[1] * std::cos(-w) + c.coefficients[2] * std::cos(-2 * w),
        c.coefficients[1] * std::sin(-w) + c.coefficients[2] * std::sin(-2 * w)
    );

    // Denominador de la función de transferencia H(z)
    std::complex<double> denominator = std::complex<double>(
        1.0 + c.coefficients[3] * std::cos(-w) + c.coefficients[4] * std::cos(-2 * w),
        c.coefficients[3] * std::sin(-w) + c.coefficients[4] * std::sin(-2 * w)
    );

    // Transferencia H en esa frecuencia
    std::complex<double> H = numerator / denominator;

    // Retornamos la magnitud (valor absoluto) de H
    return static_cast<float>(std::abs(H));
}

// Método para dibujar la interfaz gráfica
void Filter_FlowAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 30)); // Fondo gris oscuro

    // Dibujamos las curvas de respuesta EQ con colores específicos para cada banda
    g.setColour(juce::Colours::green);
    g.strokePath(lowBandPath, juce::PathStrokeType(2.0f));

    g.setColour(juce::Colours::yellow);
    g.strokePath(midBandPath, juce::PathStrokeType(2.0f));

    g.setColour(juce::Colours::red);
    g.strokePath(highBandPath, juce::PathStrokeType(2.0f));
}

// Método que ajusta el tamaño y posición de los controles cuando se redimensiona la ventana
void Filter_FlowAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Posiciona el botón bypass en la parte inferior izquierda
    bypassButton.setBounds(area.removeFromBottom(30).removeFromLeft(100));

    // Área para el visualizador EQ (arriba)
    auto visualizerArea = area.removeFromTop(150);

    // Primera fila: sliders Gain y Pan
    auto topRow = area.removeFromTop(100);
    auto controlWidth = 100;
    gainSlider.setBounds(topRow.removeFromLeft(controlWidth).reduced(10));
    panSlider.setBounds(topRow.removeFromLeft(controlWidth).reduced(10));

    // Segunda fila: sliders Low, Mid, High
    auto eqRow = area.removeFromTop(150);
    lowGainSlider.setBounds(eqRow.removeFromLeft(controlWidth).reduced(10));
    midGainSlider.setBounds(eqRow.removeFromLeft(controlWidth).reduced(10));
    highGainSlider.setBounds(eqRow.removeFromLeft(controlWidth).reduced(10));
}

// Método llamado periódicamente por el timer para actualizar la visualización
void Filter_FlowAudioProcessorEditor::timerCallback()
{
    updateEQVisualization(); // Actualiza las curvas de la EQ
    repaint();               // Fuerza que se repinte la interfaz
}

// Calcula y genera las curvas de respuesta para cada banda del EQ
void Filter_FlowAudioProcessorEditor::updateEQVisualization()
{
    auto bounds = getLocalBounds().reduced(20).removeFromTop(200);
    auto width = bounds.getWidth();

    // Limpiamos las rutas antes de dibujar
    lowBandPath.clear();
    midBandPath.clear();
    highBandPath.clear();

    const int numPoints = width; // Número de puntos a calcular para las curvas
    float sampleRate = audioProcessor.getSampleRate();

    // Recorremos cada punto para calcular la respuesta en frecuencia logarítmica
    for (int i = 0; i < numPoints; ++i)
    {
        // Frecuencia en escala logarítmica desde 20 Hz a 20 kHz aproximadamente
        float freq = 20.0f * static_cast<float>(std::pow(10.0f, 3.0f * i / (numPoints - 1)));

        // Calculamos la magnitud para cada filtro (Low, Mid, High)
        float magLow = calculateMagnitudeForFrequency(audioProcessor.eq.getLowFilter(), sampleRate, freq);
        float magMid = calculateMagnitudeForFrequency(audioProcessor.eq.getMidFilter(), sampleRate, freq);
        float magHigh = calculateMagnitudeForFrequency(audioProcessor.eq.getHighFilter(), sampleRate, freq);

        // Función para mapear la ganancia (dB) a coordenadas Y en pantalla
        auto mapToY = [&](float mag)
            {
                float dB = juce::Decibels::gainToDecibels(mag);
                // Mapea de -24 dB a +24 dB en la altura del área de dibujo
                return juce::jmap(dB, -24.0f, 24.0f, (float)bounds.getBottom(), (float)bounds.getY());
            };

        float x = bounds.getX() + (float)i; // Posición X en píxeles

        // Para el primer punto empezamos una nueva subruta, para el resto conectamos línea
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

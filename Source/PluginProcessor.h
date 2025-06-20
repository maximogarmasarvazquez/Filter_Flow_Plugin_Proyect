#pragma once  // Evita que este archivo se incluya m�s de una vez en la compilaci�n

#include <JuceHeader.h>  // Incluye todas las cabeceras principales de JUCE
#include "SimpleEQ.h"    // Incluye la definici�n de la clase SimpleEQ (tu ecualizador)

// Definici�n de la clase principal del plugin que procesa audio
class Filter_FlowAudioProcessor : public juce::AudioProcessor
{
public:
    Filter_FlowAudioProcessor();          // Constructor
    ~Filter_FlowAudioProcessor() override; // Destructor

    // M�todos esenciales que JUCE llama durante el ciclo de vida del plugin

    // Se llama antes de empezar a procesar audio. Aqu� inicializ�s cosas seg�n la tasa de muestreo y tama�o de buffer
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    // Se llama cuando el audio se detiene, para liberar recursos usados
    void releaseResources() override;

    // M�todo principal que procesa cada bloque de audio entrante
    // Aqu� aplic�s la DSP, filtros, efectos, etc.
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Crea el editor gr�fico (interfaz) del plugin
    juce::AudioProcessorEditor* createEditor() override;

    // Indica si el plugin tiene interfaz gr�fica
    bool hasEditor() const override;

    // M�todos que proveen informaci�n sobre el plugin

    // Devuelve el nombre del plugin
    const juce::String getName() const override;

    // Indica si el plugin acepta mensajes MIDI
    bool acceptsMidi() const override;

    // Indica si el plugin produce mensajes MIDI
    bool producesMidi() const override;

    // Indica si el plugin es un efecto MIDI (sin audio)
    bool isMidiEffect() const override;

    // Devuelve el tiempo en segundos que dura la cola (tail) del plugin despu�s de cortar la se�al
    double getTailLengthSeconds() const override;

    // M�todos para manejar programas o presets (configuraciones guardadas)

    // Devuelve cu�ntos programas hay
    int getNumPrograms() override;

    // Devuelve el programa actual seleccionado
    int getCurrentProgram() override;

    // Cambia el programa actual
    void setCurrentProgram(int index) override;

    // Devuelve el nombre de un programa dado su �ndice
    const juce::String getProgramName(int index) override;

    // Cambia el nombre de un programa dado su �ndice
    void changeProgramName(int index, const juce::String& newName) override;

    // M�todos para guardar y cargar el estado del plugin (los valores de los par�metros)

    // Guarda el estado del plugin en un bloque de memoria
    void getStateInformation(juce::MemoryBlock& destData) override;

    // Restaura el estado del plugin desde un bloque de memoria
    void setStateInformation(const void* data, int sizeInBytes) override;

    // M�todo que indica si un layout de buses (canales de entrada y salida) es soportado
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    // Contenedor de par�metros del plugin, maneja los valores y la automatizaci�n
    juce::AudioProcessorValueTreeState parameters;

    // Instancia del procesador de ecualizaci�n (tu DSP)
    SimpleEQ eq;

private:
    // Evita que la clase sea copiada accidentalmente y ayuda a detectar fugas de memoria en modo debug
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Filter_FlowAudioProcessor)
};
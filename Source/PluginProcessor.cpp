#include "PluginProcessor.h"
#include "PluginEditor.h"

// Constructor: inicializa el AudioProcessor y crea los parámetros
Filter_FlowAudioProcessor::Filter_FlowAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)  // Entrada estéreo si no es synth ni midi effect
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true) // Salida estéreo obligatoria
#endif
    ),
    parameters(*this, nullptr)  // Inicializa el gestor de parámetros, sin undo manager
{
    // Crea y agrega parámetros al ValueTreeState (valores que controla el plugin)

    // Parámetro "gain" rango 0.0 a 1.0, valor por defecto 0.5
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 0.5f));

    // Parámetro "pan" rango -1.0 (izquierda) a 1.0 (derecha), valor por defecto 0.0 (centro)
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("pan", "Pan", -1.0f, 1.0f, 0.0f));

    // Parámetro booleano "bypass" para activar o desactivar el efecto
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));

    // Parámetros de ganancia para las tres bandas del ecualizador (low, mid, high)
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("lowGain", "Low Gain", -24.0f, 24.0f, 0.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("midGain", "Mid Gain", -24.0f, 24.0f, 0.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("highGain", "High Gain", -24.0f, 24.0f, 0.0f));

    // Inicializa el ValueTree que guarda el estado de los parámetros
    parameters.state = juce::ValueTree("savedParams");
}

// Destructor vacío porque no hay manejo manual de memoria necesario
Filter_FlowAudioProcessor::~Filter_FlowAudioProcessor() {}

// Devuelve el nombre del plugin definido en las macros del proyecto
const juce::String Filter_FlowAudioProcessor::getName() const {
    return JucePlugin_Name;
}

// El plugin no acepta ni produce MIDI, ni es efecto MIDI
bool Filter_FlowAudioProcessor::acceptsMidi() const { return false; }
bool Filter_FlowAudioProcessor::producesMidi() const { return false; }
bool Filter_FlowAudioProcessor::isMidiEffect() const { return false; }

// No tiene cola de audio, retorna 0 segundos
double Filter_FlowAudioProcessor::getTailLengthSeconds() const { return 0.0; }

// Sólo un programa o preset disponible
int Filter_FlowAudioProcessor::getNumPrograms() { return 1; }
int Filter_FlowAudioProcessor::getCurrentProgram() { return 0; }
void Filter_FlowAudioProcessor::setCurrentProgram(int) {}
const juce::String Filter_FlowAudioProcessor::getProgramName(int) { return {}; }
void Filter_FlowAudioProcessor::changeProgramName(int, const juce::String&) {}

// Se prepara el DSP antes de empezar a procesar audio
void Filter_FlowAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Define las especificaciones para el procesamiento DSP
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;                              // Frecuencia de muestreo
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock); // Tamaño máximo del buffer
    spec.numChannels = getTotalNumOutputChannels();            // Número de canales de salida

    DBG("prepareToPlay - numChannels = " << spec.numChannels); // Debug: muestra número de canales

    eq.prepare(spec); // Prepara el ecualizador con estas especificaciones
}

// Método vacío para liberar recursos, no se usa aquí
void Filter_FlowAudioProcessor::releaseResources() {}

// Método principal que procesa el audio en cada bloque
void Filter_FlowAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // Verifica si está activo el bypass (si es así, no procesa nada)
    const bool isBypassed = *parameters.getRawParameterValue("bypass");
    if (isBypassed)
        return;

    // Obtiene el valor de ganancia y pan de los parámetros
    const float gain = *parameters.getRawParameterValue("gain");
    const float pan = *parameters.getRawParameterValue("pan");

    // Obtiene número de canales y muestras del buffer de audio
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Actualiza las ganancias del ecualizador con los valores actuales de parámetros
    eq.setGains(
        *parameters.getRawParameterValue("lowGain"),
        *parameters.getRawParameterValue("midGain"),
        *parameters.getRawParameterValue("highGain")
    );

    eq.process(buffer); // Procesa el buffer con el ecualizador

    // Calcula el balance estéreo tipo "constant power" para el PAN
    float angle = (pan + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi; // Ángulo entre 0 y π/2
    float leftGain = std::cos(angle);  // Ganancia para canal izquierdo
    float rightGain = std::sin(angle); // Ganancia para canal derecho

    // Aplica ganancia y pan según número de canales
    if (numChannels >= 2)
    {
        buffer.applyGain(0, 0, numSamples, gain * leftGain);  // Aplica ganancia al canal izquierdo
        buffer.applyGain(1, 0, numSamples, gain * rightGain); // Aplica ganancia al canal derecho
    }
    else if (numChannels == 1)
    {
        buffer.applyGain(gain); // Si es mono, solo aplica ganancia sin pan
    }
}

// Indica que el plugin tiene interfaz gráfica
bool Filter_FlowAudioProcessor::hasEditor() const { return true; }

// Crea y devuelve una instancia del editor gráfico
juce::AudioProcessorEditor* Filter_FlowAudioProcessor::createEditor()
{
    return new Filter_FlowAudioProcessorEditor(*this);
}

// Guarda el estado del plugin (parámetros) en un bloque binario
void Filter_FlowAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();                       // Copia el estado actual de los parámetros
    std::unique_ptr<juce::XmlElement> xml(state.createXml()); // Crea XML con el estado
    copyXmlToBinary(*xml, destData);                           // Convierte XML a binario para guardarlo
}

// Restaura el estado del plugin desde un bloque binario
void Filter_FlowAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes)); // Lee XML desde binario
    if (xmlState && xmlState->hasTagName(parameters.state.getType()))                // Verifica que sea válido
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));                 // Restaura los parámetros
    }
}

// Método que asegura que el plugin sólo soporte configuración estéreo
bool Filter_FlowAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Sólo permite salida estéreo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    // Entrada debe coincidir con la salida (ambas estéreo)
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true; // Layout soportado
}

// Función que crea una instancia del procesador (necesaria para JUCE)
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Filter_FlowAudioProcessor();
}

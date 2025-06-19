#include "PluginProcessor.h"
#include "PluginEditor.h"

Filter_FlowAudioProcessor::Filter_FlowAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr)
{
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 0.5f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("pan", "Pan", -1.0f, 1.0f, 0.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("lowGain", "Low Gain", -24.0f, 24.0f, 0.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("midGain", "Mid Gain", -24.0f, 24.0f, 0.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("highGain", "High Gain", -24.0f, 24.0f, 0.0f));

    parameters.state = juce::ValueTree("savedParams");
}

Filter_FlowAudioProcessor::~Filter_FlowAudioProcessor() {}

const juce::String Filter_FlowAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool Filter_FlowAudioProcessor::acceptsMidi() const { return false; }
bool Filter_FlowAudioProcessor::producesMidi() const { return false; }
bool Filter_FlowAudioProcessor::isMidiEffect() const { return false; }
double Filter_FlowAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int Filter_FlowAudioProcessor::getNumPrograms() { return 1; }
int Filter_FlowAudioProcessor::getCurrentProgram() { return 0; }
void Filter_FlowAudioProcessor::setCurrentProgram(int) {}
const juce::String Filter_FlowAudioProcessor::getProgramName(int) { return {}; }
void Filter_FlowAudioProcessor::changeProgramName(int, const juce::String&) {}

void Filter_FlowAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = getTotalNumOutputChannels();

    DBG("prepareToPlay - numChannels = " << spec.numChannels);

    eq.prepare(spec);
}

void Filter_FlowAudioProcessor::releaseResources() {}

void Filter_FlowAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    const bool isBypassed = *parameters.getRawParameterValue("bypass");
    if (isBypassed)
        return;

    const float gain = *parameters.getRawParameterValue("gain");
    const float pan = *parameters.getRawParameterValue("pan");

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Aplicar EQ
    eq.setGains(
        *parameters.getRawParameterValue("lowGain"),
        *parameters.getRawParameterValue("midGain"),
        *parameters.getRawParameterValue("highGain")
    );
    eq.process(buffer);

    // PAN (balanceo estéreo tipo "constant power")
    float angle = (pan + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi;
    float leftGain = std::cos(angle);
    float rightGain = std::sin(angle);

    if (numChannels >= 2)
    {
        buffer.applyGain(0, 0, numSamples, gain * leftGain);
        buffer.applyGain(1, 0, numSamples, gain * rightGain);
    }
    else if (numChannels == 1)
    {
        buffer.applyGain(gain); // Mono sin pan
    }
}

bool Filter_FlowAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* Filter_FlowAudioProcessor::createEditor()
{
    return new Filter_FlowAudioProcessorEditor(*this);
}

void Filter_FlowAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Filter_FlowAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(parameters.state.getType()))
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

// 🔧 Esto fuerza que el plugin sólo use layout estéreo
bool Filter_FlowAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Filter_FlowAudioProcessor();
}

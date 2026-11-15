#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    float dbToGain(float db) { return juce::Decibels::decibelsToGain(db); }
}

VoxHarmonicRiderAudioProcessor::APVTS::ParameterLayout
VoxHarmonicRiderAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout p;
    auto addFloat = [&p](const char* id, const char* name, float lo, float hi, float def, float step = 0.0f)
    {
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{id, 1}, name,
            juce::NormalisableRange<float>(lo, hi, step), def));
    };
    auto addBool = [&p](const char* id, const char* name, bool def)
    {
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id, 1}, name, def));
    };

    addFloat("input", "Input", -24.0f, 24.0f, 0.0f);
    addFloat("fundamental", "Fundamental", -18.0f, 18.0f, 0.0f);
    addFloat("second", "Second Harmonic", -18.0f, 18.0f, 0.0f);
    addFloat("third", "Third Harmonic", -18.0f, 18.0f, 0.0f);
    addFloat("nth", "Nth Harmonic", -18.0f, 18.0f, 0.0f);
    addFloat("nthIndex", "Nth Harmonic Select", 4.0f, 14.0f, 4.0f, 1.0f);
    addFloat("lowCut", "Low Cut", 20.0f, 500.0f, 70.0f);
    addFloat("output", "Output", -24.0f, 12.0f, 0.0f);
    addFloat("dynThreshold", "Dynamics Threshold", -48.0f, 0.0f, -18.0f);
    addFloat("dynStrength", "Dynamics Strength", 0.0f, 100.0f, 0.0f);
    addBool("dynFund", "Fundamental Dynamics", false);
    addBool("dynSecond", "Second Dynamics", false);
    addBool("dynThird", "Third Dynamics", false);
    addBool("dynNth", "Nth Dynamics", false);
    addBool("soloFund", "Fundamental Solo", false);
    addBool("soloSecond", "Second Solo", false);
    addBool("soloThird", "Third Solo", false);
    addBool("soloNth", "Nth Solo", false);
    addBool("live", "Live Mode", true);
    return p;
}

VoxHarmonicRiderAudioProcessor::VoxHarmonicRiderAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

bool VoxHarmonicRiderAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& in = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void VoxHarmonicRiderAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    spec = { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumOutputChannels() };
    lowCutL.prepare(spec);
    lowCutR.prepare(spec);

    for (int i = 0; i < 4; ++i)
    {
        bandL[i].prepare(spec);
        bandR[i].prepare(spec);
    }

    lowCutL.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    lowCutR.setType(juce::dsp::StateVariableTPTFilterType::highpass);

    for (int i = 0; i < 4; ++i)
    {
        bandL[i].setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        bandR[i].setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    }

    detectedHz = 220.0f;
    pitchUpdateCounter = 0;
    for (float& e : envelope)
        e = 0.0f;
}

void VoxHarmonicRiderAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int n = buffer.getNumSamples();
    if (channels == 0)
        return;

    const float inputGain = dbToGain(parameters.getRawParameterValue("input")->load());
    const float outputGain = dbToGain(parameters.getRawParameterValue("output")->load());
    const float lowCut = parameters.getRawParameterValue("lowCut")->load();
    const float nthIndex = parameters.getRawParameterValue("nthIndex")->load();
    const float threshold = dbToGain(parameters.getRawParameterValue("dynThreshold")->load());
    const float dynAmount = parameters.getRawParameterValue("dynStrength")->load() * 0.01f;

    const float gains[4] = {
        parameters.getRawParameterValue("fundamental")->load(),
        parameters.getRawParameterValue("second")->load(),
        parameters.getRawParameterValue("third")->load(),
        parameters.getRawParameterValue("nth")->load()
    };
    const bool dyn[4] = {
        parameters.getRawParameterValue("dynFund")->load() > 0.5f,
        parameters.getRawParameterValue("dynSecond")->load() > 0.5f,
        parameters.getRawParameterValue("dynThird")->load() > 0.5f,
        parameters.getRawParameterValue("dynNth")->load() > 0.5f
    };
    const bool solo[4] = {
        parameters.getRawParameterValue("soloFund")->load() > 0.5f,
        parameters.getRawParameterValue("soloSecond")->load() > 0.5f,
        parameters.getRawParameterValue("soloThird")->load() > 0.5f,
        parameters.getRawParameterValue("soloNth")->load() > 0.5f
    };

    lowCutL.setCutoffFrequency(lowCut);
    lowCutR.setCutoffFrequency(lowCut);

    const int selectedSolo = solo[3] ? 3 : (solo[2] ? 2 : (solo[1] ? 1 : (solo[0] ? 0 : -1)));

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* x = buffer.getWritePointer(ch);
        auto& low = (ch == 0 ? lowCutL : lowCutR);
        auto* filters = (ch == 0 ? bandL : bandR);

        for (int i = 0; i < n; ++i)
        {
            const float dry = x[i] * inputGain;
            const float filtered = low.processSample(0, dry);

            // Deliberately simple independent pitch-following approximation.
            if (ch == 0 && ++pitchUpdateCounter >= 256)
            {
                pitchUpdateCounter = 0;
                const float estimate = juce::jlimit(65.0f, 1000.0f, 80.0f + 420.0f * std::abs(filtered));
                detectedHz = 0.985f * detectedHz + 0.015f * estimate;
            }

            const float fundamental = juce::jlimit(55.0f, 1000.0f, detectedHz);
            const float hz[4] = {
                fundamental,
                fundamental * 2.0f,
                fundamental * 3.0f,
                juce::jlimit(120.0f, 12000.0f, fundamental * nthIndex)
            };

            float wet = 0.0f;
            float selected = 0.0f;

            for (int b = 0; b < 4; ++b)
            {
                const float centre = juce::jlimit(30.0f, (float)(spec.sampleRate * 0.45), hz[b]);
                filters[b].setCutoffFrequency(centre);
                const float partial = filters[b].processSample(0, filtered);

                const float level = std::abs(partial);
                const float coefficient = level > envelope[b] ? 0.08f : 0.0025f;
                envelope[b] += coefficient * (level - envelope[b]);

                float dynGain = 1.0f;
                if (dyn[b] && envelope[b] > threshold)
                {
                    const float over = (envelope[b] - threshold) / juce::jmax(0.001f, threshold);
                    dynGain = 1.0f - dynAmount * juce::jlimit(0.0f, 1.0f, over);
                }

                const float processed = partial * dbToGain(gains[b]) * dynGain;
                wet += processed;
                if (selectedSolo == b)
                    selected = partial;
            }

            const float y = selectedSolo >= 0 ? selected : (filtered + wet * 0.35f);
            x[i] = std::tanh(y * outputGain);
        }
    }
}

juce::AudioProcessorEditor* VoxHarmonicRiderAudioProcessor::createEditor()
{
    return new VoxHarmonicRiderAudioProcessorEditor(*this);
}

void VoxHarmonicRiderAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void VoxHarmonicRiderAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoxHarmonicRiderAudioProcessor();
}

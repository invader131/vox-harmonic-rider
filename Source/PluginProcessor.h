#pragma once
#include <JuceHeader.h>

class VoxHarmonicRiderAudioProcessor final : public juce::AudioProcessor
{
public:
    VoxHarmonicRiderAudioProcessor();
    ~VoxHarmonicRiderAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;

private:
    static APVTS::ParameterLayout createParameterLayout();
    juce::dsp::ProcessSpec spec {};
    juce::dsp::StateVariableTPTFilter<float> lowCutL, lowCutR;
    juce::dsp::StateVariableTPTFilter<float> bandL[4], bandR[4];
    float envelope[4] { 0, 0, 0, 0 };
    float detectedHz = 220.0f;
    int pitchUpdateCounter = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxHarmonicRiderAudioProcessor)
};

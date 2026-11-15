#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VoxHarmonicRiderAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit VoxHarmonicRiderAudioProcessorEditor(VoxHarmonicRiderAudioProcessor&);
    ~VoxHarmonicRiderAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using APVTS = VoxHarmonicRiderAudioProcessor::APVTS;
    struct SliderControl {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<APVTS::SliderAttachment> attachment;
    };
    struct ToggleControl {
        juce::ToggleButton button;
        std::unique_ptr<APVTS::ButtonAttachment> attachment;
    };
    VoxHarmonicRiderAudioProcessor& processor;
    std::vector<std::unique_ptr<SliderControl>> sliders;
    std::vector<std::unique_ptr<ToggleControl>> toggles;
    void addSlider(const juce::String& id, const juce::String& caption);
    void addToggle(const juce::String& id, const juce::String& caption);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoxHarmonicRiderAudioProcessorEditor)
};

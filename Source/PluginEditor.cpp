#include "PluginEditor.h"

VoxHarmonicRiderAudioProcessorEditor::VoxHarmonicRiderAudioProcessorEditor(VoxHarmonicRiderAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(760, 470);
    addSlider("input", "INPUT");
    addSlider("lowCut", "LOW CUT");
    addSlider("fundamental", "FUNDAMENTAL");
    addSlider("second", "2ND HARMONIC");
    addSlider("third", "3RD HARMONIC");
    addSlider("nth", "NTH HARMONIC");
    addSlider("nthIndex", "NTH SELECT");
    addSlider("dynThreshold", "DYN THRESHOLD");
    addSlider("dynStrength", "DYN STRENGTH");
    addSlider("output", "OUTPUT");
    addToggle("soloFund", "SOLO FUND");
    addToggle("soloSecond", "SOLO 2ND");
    addToggle("soloThird", "SOLO 3RD");
    addToggle("soloNth", "SOLO NTH");
    addToggle("dynFund", "DYN FUND");
    addToggle("dynSecond", "DYN 2ND");
    addToggle("dynThird", "DYN 3RD");
    addToggle("dynNth", "DYN NTH");
    addToggle("live", "LIVE MODE");

    // Controls are created after setSize(); run layout again so they receive bounds.
    resized();
}

void VoxHarmonicRiderAudioProcessorEditor::addSlider(const juce::String& id, const juce::String& caption)
{
    auto c = std::make_unique<SliderControl>();
    c->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    c->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
    c->label.setText(caption, juce::dontSendNotification);
    c->label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(c->slider);
    addAndMakeVisible(c->label);
    c->attachment = std::make_unique<APVTS::SliderAttachment>(processor.parameters, id, c->slider);
    sliders.push_back(std::move(c));
}

void VoxHarmonicRiderAudioProcessorEditor::addToggle(const juce::String& id, const juce::String& caption)
{
    auto c = std::make_unique<ToggleControl>();
    c->button.setButtonText(caption);
    addAndMakeVisible(c->button);
    c->attachment = std::make_unique<APVTS::ButtonAttachment>(processor.parameters, id, c->button);
    toggles.push_back(std::move(c));
}

void VoxHarmonicRiderAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff11151b));
    g.setColour(juce::Colour(0xffe8edf4));
    g.setFont(juce::Font(25.0f, juce::Font::bold));
    g.drawText("VOX HARMONIC RIDER", 24, 14, getWidth() - 48, 34, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff8493a5));
    g.setFont(12.0f);
    g.drawText("Independent harmonic processor - JUCE 8", 26, 48, getWidth() - 52, 20, juce::Justification::centredLeft);
}

void VoxHarmonicRiderAudioProcessorEditor::resized()
{
    const int margin = 22;
    const int top = 82;
    const int sliderW = 118;
    const int sliderH = 126;
    for (size_t i = 0; i < sliders.size(); ++i)
    {
        const int col = (int)(i % 5);
        const int row = (int)(i / 5);
        auto& c = *sliders[i];
        c.label.setBounds(margin + col * 142, top + row * 142, sliderW, 20);
        c.slider.setBounds(margin + col * 142, top + 18 + row * 142, sliderW, sliderH);
    }
    const int toggleY = 386;
    for (size_t i = 0; i < toggles.size(); ++i)
    {
        const int col = (int)(i % 5);
        const int row = (int)(i / 5);
        toggles[i]->button.setBounds(margin + col * 142, toggleY + row * 28, 132, 24);
    }
}

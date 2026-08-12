#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ParameterIDs.h"

StaticCathedralAudioProcessorEditor::StaticCathedralAudioProcessorEditor(StaticCathedralAudioProcessor& p)
    : AudioProcessorEditor(&p), ownerProcessor(p),
      tooltipText("StaticCathedral: every reverb parameter is exposed as a named, tooltipped host control.")
{
    setResizeLimits(minimumWidth, minimumHeight, defaultWidth * 2, defaultHeight * 2);
    setResizable(true, true);
    setName("StaticCathedral editor");
    setComponentID("staticcathedral-editor");
    setTitle("StaticCathedral");
    setDescription("StaticCathedral monochrome 8-bit custom editor");
    setWantsKeyboardFocus(true);
    setLookAndFeel(&lookAndFeel);

    for (std::size_t i = 0; i < staticcathedral::parameters::all.size(); ++i)
    {
        auto& slider = sliders[i];
        auto& label = labels[i];
        const auto* id = staticcathedral::parameters::all[i];
        const auto* name = staticcathedral::parameters::names[i];
        const auto* tip = staticcathedral::parameters::tooltips[i];

        slider.setName(name);
        slider.setComponentID(juce::String("staticcathedral-control-") + id);
        slider.setTitle(name);
        slider.setDescription(tip);
        slider.setTooltip(tip);
        slider.setLookAndFeel(&lookAndFeel);
        ehl::juce_design::styleSlider(slider);
        addAndMakeVisible(slider);

        label.setText(name, juce::dontSendNotification);
        label.setName(juce::String(name) + " label");
        label.setComponentID(juce::String("staticcathedral-label-") + id);
        label.setTitle(juce::String(name) + " label");
        label.setDescription(tip);
        label.setTooltip(tip);
        label.setLookAndFeel(&lookAndFeel);
        ehl::juce_design::styleLabel(label);
        addAndMakeVisible(label);

        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(ownerProcessor.parameters, id, slider);
    }

    setSize(defaultWidth, defaultHeight);
}

StaticCathedralAudioProcessorEditor::~StaticCathedralAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    for (auto& label : labels)
        label.setLookAndFeel(nullptr);
    for (auto& slider : sliders)
        slider.setLookAndFeel(nullptr);
}

void StaticCathedralAudioProcessorEditor::paint(juce::Graphics& g)
{
    ehl::juce_design::paintEditorChrome(g, getLocalBounds(), "StaticCathedral", "REVERB");
}

void StaticCathedralAudioProcessorEditor::resized()
{
    for (std::size_t i = 0; i < sliders.size(); ++i)
        ehl::juce_design::layoutLabelledControl(labels[i], sliders[i],
                                                ehl::juce_design::controlCell(getLocalBounds(), i));
}

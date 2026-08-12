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

    for (std::size_t i = 0; i < staticcathedral::parameters::all.size(); ++i)
    {
        auto& slider = sliders[i];
        auto& label = labels[i];
        const auto* id = staticcathedral::parameters::all[i];
        const auto* name = staticcathedral::parameters::names[i];
        const auto* tip = staticcathedral::parameters::tooltips[i];

        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 24);
        slider.setName(name);
        slider.setComponentID(juce::String("staticcathedral-control-") + id);
        slider.setTitle(name);
        slider.setDescription(tip);
        slider.setTooltip(tip);
        slider.setWantsKeyboardFocus(true);
        slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff8a8a86));
        slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2a2a2a));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff2f2f0));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff2f2f0));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff050505));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff8a8a86));
        addAndMakeVisible(slider);

        label.setText(name, juce::dontSendNotification);
        label.setName(juce::String(name) + " label");
        label.setComponentID(juce::String("staticcathedral-label-") + id);
        label.setTitle(juce::String(name) + " label");
        label.setDescription(tip);
        label.setTooltip(tip);
        label.attachToComponent(&slider, false);
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colour(0xfff2f2f0));
        addAndMakeVisible(label);

        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(ownerProcessor.parameters, id, slider);
    }

    setSize(defaultWidth, defaultHeight);
}

void StaticCathedralAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds();
    g.fillAll(juce::Colour(0xff050505));

    g.setColour(juce::Colour(0xfff2f2f0));
    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    g.drawText("StaticCathedral", 32, 16, area.getWidth() - 64, 32, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff8a8a86));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("REVERB", 32, 48, area.getWidth() - 64, 16, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawHorizontalLine(72, 32.0f, static_cast<float>(area.getWidth() - 32));
}

void StaticCathedralAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(32);
    area.removeFromTop(48);

    const int rows = 6;
    const int columns = 2;
    const int rowHeight = area.getHeight() / rows;
    const int colWidth = area.getWidth() / columns;

    for (std::size_t i = 0; i < sliders.size(); ++i)
    {
        const int row = static_cast<int>(i) % rows;
        const int column = static_cast<int>(i) / rows;
        auto cell = juce::Rectangle<int>(area.getX() + column * colWidth,
                                         area.getY() + row * rowHeight,
                                         colWidth,
                                         rowHeight).reduced(8, 8);
        labels[i].setBounds(cell.removeFromLeft(128));
        sliders[i].setBounds(cell);
    }
}

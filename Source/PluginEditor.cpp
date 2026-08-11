#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ParameterIDs.h"

StaticCathedralAudioProcessorEditor::StaticCathedralAudioProcessorEditor(StaticCathedralAudioProcessor& p)
    : AudioProcessorEditor(&p), ownerProcessor(p),
      tooltipText("StaticCathedral: every reverb parameter is exposed as a named, tooltipped host control.")
{
    setSize(defaultWidth, defaultHeight);
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
        slider.setColour(juce::Slider::trackColourId, juce::Colour(0xffd8d8d8));
        slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff242424));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff0f0f0));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff050505));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff707070));
        addAndMakeVisible(slider);

        label.setText(name, juce::dontSendNotification);
        label.setName(juce::String(name) + " label");
        label.setComponentID(juce::String("staticcathedral-label-") + id);
        label.setTitle(juce::String(name) + " label");
        label.setDescription(tip);
        label.setTooltip(tip);
        label.attachToComponent(&slider, false);
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colour(0xfff0f0f0));
        addAndMakeVisible(label);

        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(ownerProcessor.parameters, id, slider);
    }
}

void StaticCathedralAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds();
    g.fillAll(juce::Colour(0xff050505));

    const auto grid = 8;
    g.setColour(juce::Colour(0xff202020));
    for (int x = 0; x < area.getWidth(); x += grid)
        g.drawVerticalLine(x, 0.0f, static_cast<float>(area.getHeight()));
    for (int y = 0; y < area.getHeight(); y += grid)
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(area.getWidth()));

    const float size = ownerProcessor.parameters.getRawParameterValue(staticcathedral::parameters::size)->load();
    const float decay = ownerProcessor.parameters.getRawParameterValue(staticcathedral::parameters::decay)->load();
    const float diffusion = ownerProcessor.parameters.getRawParameterValue(staticcathedral::parameters::diffusion)->load();
    const float smear = ownerProcessor.parameters.getRawParameterValue(staticcathedral::parameters::cathedralSmear)->load();
    const float freeze = ownerProcessor.parameters.getRawParameterValue(staticcathedral::parameters::freeze)->load();

    g.setColour(juce::Colour(0xffe8e8e8));
    g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    g.drawText("StaticCathedral", 32, 24, area.getWidth() - 64, 48, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(16.0f));
    g.drawText("jp.ehl.staticcathedral / StCt", 34, 74, area.getWidth() - 68, 24, juce::Justification::centredLeft);

    const int left = area.getWidth() - 240;
    const int base = 130;
    const int columns = 8;
    const int archWidth = 18;
    for (int i = 0; i < columns; ++i)
    {
        const int x = left + i * 26;
        const int h = 64 + static_cast<int>((size * 48.0f) + (i % 3) * decay * 24.0f);
        g.setColour(juce::Colour(i % 2 == 0 ? 0xffd6d6d6 : 0xff9a9a9a));
        g.fillRect(x, base + 150 - h, archWidth, h);
        g.setColour(juce::Colour(0xff050505));
        g.fillRect(x + 4, base + 152 - h, archWidth - 8, 10 + static_cast<int>(diffusion * 22.0f));
    }

    g.setColour(juce::Colour(0xfff2f2f2));
    for (int x = 32; x < area.getWidth() - 32; x += 24)
    {
        const int h = 16 + ((x / 24) % 9) * 8 + static_cast<int>(smear * 18.0f) + static_cast<int>(freeze * 12.0f);
        g.fillRect(x, area.getHeight() - 48 - h, 8, h);
    }
}

void StaticCathedralAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(32);
    area.removeFromTop(96);
    area.removeFromRight(260);

    const int rowHeight = 32;
    const int gap = 6;
    for (std::size_t i = 0; i < sliders.size(); ++i)
    {
        auto row = area.removeFromTop(rowHeight);
        labels[i].setBounds(row.removeFromLeft(144));
        sliders[i].setBounds(row);
        area.removeFromTop(gap);
    }
}

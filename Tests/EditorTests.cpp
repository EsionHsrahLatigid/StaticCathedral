#include "TestSupport.h"
#include "ParameterIDs.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"

#include <ehl/juce_design/EhlDesign.h>

#include <juce_events/juce_events.h>

#include <algorithm>

namespace
{
void checkSimpleHeaderOnlyPaint(juce::AudioProcessorEditor& editor)
{
    const auto background = ehl::juce_design::Palette::ink();
    const auto divider = ehl::juce_design::Palette::low();

    juce::Image image(juce::Image::RGB, editor.getWidth(), editor.getHeight(), true);
    juce::Graphics g(image);
    editor.paint(g);

    bool headerHasInk = false;
    bool middleBandIsBackground = true;
    bool dividerIsExact = true;
    bool bodyIsBackground = true;
    bool neutral = true;

    for (int y = 0; y < image.getHeight(); ++y)
    {
        for (int x = 0; x < image.getWidth(); ++x)
        {
            const auto pixel = image.getPixelAt(x, y);
            // The EHL paper/mid tones are warm neutral (#F2F2F0/#8A8A86), so
            // monochrome allows a four-count channel spread while rejecting accents.
            const auto minChannel = std::min({ pixel.getRed(), pixel.getGreen(), pixel.getBlue() });
            const auto maxChannel = std::max({ pixel.getRed(), pixel.getGreen(), pixel.getBlue() });
            neutral = neutral && static_cast<int>(maxChannel) - static_cast<int>(minChannel) <= 4;

            if (y < 48)
                headerHasInk = headerHasInk || pixel != background;
            else if (y < ehl::juce_design::Metrics::dividerY)
                middleBandIsBackground = middleBandIsBackground && pixel == background;
            else if (y == ehl::juce_design::Metrics::dividerY)
            {
                const bool onDivider = x >= ehl::juce_design::Metrics::margin
                                    && x < image.getWidth() - ehl::juce_design::Metrics::margin;
                dividerIsExact = dividerIsExact && pixel == (onDivider ? divider : background);
            }
            else if (y >= ehl::juce_design::Metrics::headerHeight)
                bodyIsBackground = bodyIsBackground && pixel == background;
        }
    }

    test_support::check(image.getPixelAt(0, 0) == background, "paint background is #050505");
    test_support::check(headerHasInk, "paint keeps product identity above y=48");
    test_support::check(middleBandIsBackground, "paint keeps y=48..55 empty before the divider");
    test_support::check(dividerIsExact, "paint draws the shared divider at y=56 from x=16 through width-17");
    test_support::check(bodyIsBackground, "paint draws no decorative motif, grid, panel, meter, or visualizer at y>=64");
    test_support::check(neutral, "paint stays in the neutral monochrome ramp");
}

void checkControlContract(juce::AudioProcessorEditor& editor, const char* parameterID, std::size_t index)
{
    const juce::String id(parameterID);
    auto* label = dynamic_cast<juce::Label*>(editor.findChildWithID("staticcathedral-label-" + id));
    auto* control = dynamic_cast<juce::Slider*>(editor.findChildWithID("staticcathedral-control-" + id));
    test_support::check(label != nullptr, "label exists: " + id.toStdString());
    test_support::check(control != nullptr, "control exists: " + id.toStdString());

    const auto expected = ehl::juce_design::labelledControlBounds(ehl::juce_design::controlCell(editor.getLocalBounds(), index));
    test_support::check(label->getBounds() == expected.label, "label uses shared grid: " + id.toStdString());
    test_support::check(control->getBounds() == expected.control, "control uses shared grid: " + id.toStdString());
    test_support::check(control->getY() >= ehl::juce_design::Metrics::headerHeight, "control starts at or below y=64: " + id.toStdString());
    test_support::check(control->getRight() <= editor.getWidth(), "control fits editor width: " + id.toStdString());
    test_support::check(control->getBottom() <= editor.getHeight(), "control fits editor height: " + id.toStdString());
    test_support::check(control->getSliderStyle() == juce::Slider::LinearHorizontal, "control uses shared slider style: " + id.toStdString());
    test_support::check(control->getTextBoxWidth() == ehl::juce_design::Metrics::valueWidth, "control uses shared value width: " + id.toStdString());
    test_support::check(control->findColour(juce::Slider::thumbColourId) == ehl::juce_design::Palette::paper(), "control uses shared paper thumb: " + id.toStdString());
    test_support::check(control->findColour(juce::Slider::trackColourId) == ehl::juce_design::Palette::mid(), "control uses shared mid track: " + id.toStdString());
    test_support::check(control->findColour(juce::Slider::backgroundColourId) == ehl::juce_design::Palette::low(), "control uses shared low background: " + id.toStdString());
}

void checkAllControlContracts(juce::AudioProcessorEditor& editor)
{
    for (std::size_t i = 0; i < staticcathedral::parameters::all.size(); ++i)
        checkControlContract(editor, staticcathedral::parameters::all[i], i);
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juce;
    return test_support::run("staticcathedral_editor_tests", [] {
        StaticCathedralAudioProcessor processor;
        std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
        auto* custom = dynamic_cast<StaticCathedralAudioProcessorEditor*>(editor.get());
        test_support::check(custom != nullptr, "custom editor type, not GenericAudioProcessorEditor");
        test_support::check(dynamic_cast<juce::GenericAudioProcessorEditor*>(editor.get()) == nullptr, "not GenericAudioProcessorEditor");
        test_support::check(editor->getWidth() == ehl::juce_design::Metrics::defaultWidth, "default width");
        test_support::check(editor->getHeight() == ehl::juce_design::Metrics::defaultHeight, "default height");
        test_support::check(StaticCathedralAudioProcessorEditor::minimumWidth == ehl::juce_design::Metrics::minimumWidth, "minimum width");
        test_support::check(StaticCathedralAudioProcessorEditor::minimumHeight == ehl::juce_design::Metrics::minimumHeight, "minimum height");
        test_support::check(editor->getComponentID() == "staticcathedral-editor", "editor component id");
        test_support::check(editor->getName().isNotEmpty(), "accessible name");
        test_support::check(custom->getTooltip().isNotEmpty(), "editor tooltip");
        test_support::check(editor->getWantsKeyboardFocus(), "keyboard focus");

        for (const auto* id : staticcathedral::parameters::all)
        {
            auto* control = editor->findChildWithID(juce::String("staticcathedral-control-") + id);
            const std::string suffix(id);
            test_support::check(control != nullptr, "control exists: " + suffix);
            auto* slider = dynamic_cast<juce::Slider*>(control);
            test_support::check(slider != nullptr, "control is slider: " + suffix);
            test_support::check(control->getName().isNotEmpty(), "control has name: " + suffix);
            test_support::check(slider->getTooltip().isNotEmpty(), "control has tooltip: " + suffix);
            test_support::check(control->getWantsKeyboardFocus(), "control accepts keyboard focus: " + suffix);
        }

        editor->setBounds(0, 0, ehl::juce_design::Metrics::defaultWidth, ehl::juce_design::Metrics::defaultHeight);
        checkAllControlContracts(*editor);
        checkSimpleHeaderOnlyPaint(*editor);

        editor->setBounds(0, 0, ehl::juce_design::Metrics::minimumWidth, ehl::juce_design::Metrics::minimumHeight);
        checkAllControlContracts(*editor);
        checkSimpleHeaderOnlyPaint(*editor);
    });
}

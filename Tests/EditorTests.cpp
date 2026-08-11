#include "TestSupport.h"
#include "ParameterIDs.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"

#include <juce_events/juce_events.h>

int main()
{
    juce::ScopedJuceInitialiser_GUI juce;
    return test_support::run("staticcathedral_editor_tests", [] {
        StaticCathedralAudioProcessor processor;
        std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
        auto* custom = dynamic_cast<StaticCathedralAudioProcessorEditor*>(editor.get());
        test_support::check(custom != nullptr, "custom editor type, not GenericAudioProcessorEditor");
        test_support::check(dynamic_cast<juce::GenericAudioProcessorEditor*>(editor.get()) == nullptr, "not GenericAudioProcessorEditor");
        test_support::check(editor->getWidth() == StaticCathedralAudioProcessorEditor::defaultWidth, "default width");
        test_support::check(editor->getHeight() == StaticCathedralAudioProcessorEditor::defaultHeight, "default height");
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

        juce::Image image(juce::Image::RGB, 320, 200, true);
        juce::Graphics g(image);
        editor->setBounds(0, 0, image.getWidth(), image.getHeight());
        editor->paint(g);
        const auto first = image.getPixelAt(0, 0);
        bool varied = false;
        bool grayscale = true;
        for (int y = 0; y < image.getHeight(); y += 16)
        {
            for (int x = 0; x < image.getWidth(); x += 16)
            {
                const auto pixel = image.getPixelAt(x, y);
                varied = varied || pixel != first;
                grayscale = grayscale
                    && pixel.getRed() == pixel.getGreen()
                    && pixel.getGreen() == pixel.getBlue();
            }
        }
        test_support::check(varied, "software paint uses procedural cathedral/decay motif");
        test_support::check(grayscale, "paint samples stay in grayscale palette");
    });
}

#pragma once

#include "ParameterIDs.h"

#include <ehl/juce_design/EhlDesign.h>

#include <array>
#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>

class StaticCathedralAudioProcessor;

class StaticCathedralAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                  private juce::Timer
{
public:
    explicit StaticCathedralAudioProcessorEditor(StaticCathedralAudioProcessor&);
    ~StaticCathedralAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    juce::String getTooltip() { return tooltipText; }

    static constexpr int defaultWidth = ehl::juce_design::Metrics::defaultWidth;
    static constexpr int defaultHeight = ehl::juce_design::Metrics::defaultHeight;
    static constexpr int minimumWidth = ehl::juce_design::Metrics::minimumWidth;
    static constexpr int minimumHeight = ehl::juce_design::Metrics::minimumHeight;

private:
    friend struct EditorTestAccess;

    void timerCallback() override;

    StaticCathedralAudioProcessor& ownerProcessor;
    juce::TooltipWindow tooltipWindow { this, 700 };
    juce::String tooltipText;
    ehl::juce_design::LookAndFeel lookAndFeel;
    ehl::juce_design::ParameterDisplay parameterDisplay { ehl::juce_design::DisplayKind::reverb };
    std::array<juce::Slider, staticcathedral::parameters::all.size()> sliders;
    std::array<juce::Label, staticcathedral::parameters::all.size()> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, staticcathedral::parameters::all.size()> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaticCathedralAudioProcessorEditor)
};

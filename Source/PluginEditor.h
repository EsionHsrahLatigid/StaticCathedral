#pragma once

#include "ParameterIDs.h"

#include <array>
#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>

class StaticCathedralAudioProcessor;

class StaticCathedralAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit StaticCathedralAudioProcessorEditor(StaticCathedralAudioProcessor&);
    ~StaticCathedralAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    juce::String getTooltip() { return tooltipText; }

    static constexpr int defaultWidth = 960;
    static constexpr int defaultHeight = 544;
    static constexpr int minimumWidth = 720;
    static constexpr int minimumHeight = 432;

private:
    StaticCathedralAudioProcessor& ownerProcessor;
    juce::TooltipWindow tooltipWindow { this, 700 };
    juce::String tooltipText;
    std::array<juce::Slider, staticcathedral::parameters::all.size()> sliders;
    std::array<juce::Label, staticcathedral::parameters::all.size()> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, staticcathedral::parameters::all.size()> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaticCathedralAudioProcessorEditor)
};

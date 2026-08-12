#include "TestSupport.h"
#include "ParameterIDs.h"
#include "PluginProcessor.h"

#include <juce_events/juce_events.h>
#include <cmath>
#include <memory>

namespace
{
juce::AudioProcessor::BusesLayout layoutFor(const juce::AudioChannelSet& input, const juce::AudioChannelSet& output)
{
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add(input);
    layout.outputBuses.add(output);
    return layout;
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juce;
    return test_support::run("staticcathedral_plugin_tests", [] {
        auto processor = std::make_unique<StaticCathedralAudioProcessor>();
        test_support::check(processor->getName() == "StaticCathedral", "product name");
        test_support::check(!processor->acceptsMidi(), "effect does not require MIDI");
        test_support::check(!processor->isMidiEffect(), "audio effect");
        test_support::check(processor->getLatencySamples() == 0, "zero latency");
        test_support::check(processor->getTailLengthSeconds() >= 10.0, "reverb declares a long tail");

        test_support::check(processor->isBusesLayoutSupported(layoutFor(juce::AudioChannelSet::mono(), juce::AudioChannelSet::mono())),
            "mono to mono bus supported");
        test_support::check(processor->isBusesLayoutSupported(layoutFor(juce::AudioChannelSet::stereo(), juce::AudioChannelSet::stereo())),
            "stereo to stereo bus supported");
        test_support::check(!processor->isBusesLayoutSupported(layoutFor(juce::AudioChannelSet::mono(), juce::AudioChannelSet::stereo())),
            "mono to stereo bus rejected");
        test_support::check(!processor->isBusesLayoutSupported(layoutFor(juce::AudioChannelSet::stereo(), juce::AudioChannelSet::mono())),
            "stereo to mono bus rejected");

        for (const auto* id : staticcathedral::parameters::all)
            test_support::check(processor->parameters.getParameter(id) != nullptr, std::string("APVTS parameter exists: ") + id);

        auto* decay = processor->parameters.getParameter(staticcathedral::parameters::decay);
        decay->setValueNotifyingHost(decay->convertTo0to1(0.81f));
        juce::MemoryBlock state;
        processor->getStateInformation(state);
        decay->setValueNotifyingHost(decay->convertTo0to1(0.1f));
        processor->setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        test_support::check(std::abs(processor->parameters.getRawParameterValue(staticcathedral::parameters::decay)->load() - 0.81f) < 0.01f,
            "state round-trip restores decay");

        const char invalid[] = "not xml";
        processor->setStateInformation(invalid, static_cast<int>(sizeof(invalid)));
        test_support::check(std::isfinite(processor->parameters.getRawParameterValue(staticcathedral::parameters::decay)->load()),
            "invalid state ignored safely");

        processor->prepareToPlay(48000.0, 64);
        juce::AudioBuffer<float> stereoBuffer(2, 64);
        for (int i = 0; i < 64; ++i)
        {
            stereoBuffer.setSample(0, i, i == 0 ? 1.0f : 0.0f);
            stereoBuffer.setSample(1, i, i == 0 ? -1.0f : 0.0f);
        }
        juce::MidiBuffer midi;
        processor->processBlock(stereoBuffer, midi);
        for (int ch = 0; ch < stereoBuffer.getNumChannels(); ++ch)
            for (int i = 0; i < stereoBuffer.getNumSamples(); ++i)
                test_support::check(std::isfinite(stereoBuffer.getSample(ch, i)), "processed stereo samples finite");

        auto monoProcessor = std::make_unique<StaticCathedralAudioProcessor>();
        test_support::check(monoProcessor->setBusesLayout(layoutFor(juce::AudioChannelSet::mono(), juce::AudioChannelSet::mono())),
            "mono processor bus layout applied");
        monoProcessor->prepareToPlay(44100.0, 32);
        juce::AudioBuffer<float> monoBuffer(1, 32);
        monoBuffer.setSample(0, 0, 0.7f);
        monoProcessor->processBlock(monoBuffer, midi);
        for (int i = 0; i < monoBuffer.getNumSamples(); ++i)
            test_support::check(std::isfinite(monoBuffer.getSample(0, i)), "processed mono samples finite");
    });
}

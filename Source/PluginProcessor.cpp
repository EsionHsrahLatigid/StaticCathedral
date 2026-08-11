#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

#include <cmath>
#include <memory>

StaticCathedralAudioProcessor::StaticCathedralAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout StaticCathedralAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    const std::array<float, staticcathedral::parameters::all.size()> defaults {
        0.62f, 0.72f, 0.18f, 0.78f, 0.42f, 0.24f, 0.20f, 0.58f, 0.0f, 0.72f, 0.84f, 0.42f
    };
    for (std::size_t i = 0; i < staticcathedral::parameters::all.size(); ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            staticcathedral::parameters::all[i],
            staticcathedral::parameters::names[i],
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f),
            defaults[i]));
    }
    return { params.begin(), params.end() };
}

void StaticCathedralAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dsp.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

bool StaticCathedralAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    return mainIn == mainOut;
}

void StaticCathedralAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    staticcathedral::dsp::ReverbParameters target;
    target.size = parameters.getRawParameterValue(staticcathedral::parameters::size)->load();
    target.decay = parameters.getRawParameterValue(staticcathedral::parameters::decay)->load();
    target.preDelay = parameters.getRawParameterValue(staticcathedral::parameters::preDelay)->load();
    target.diffusion = parameters.getRawParameterValue(staticcathedral::parameters::diffusion)->load();
    target.dampingTilt = parameters.getRawParameterValue(staticcathedral::parameters::dampingTilt)->load();
    target.modulationDepth = parameters.getRawParameterValue(staticcathedral::parameters::modulationDepth)->load();
    target.modulationRate = parameters.getRawParameterValue(staticcathedral::parameters::modulationRate)->load();
    target.cathedralSmear = parameters.getRawParameterValue(staticcathedral::parameters::cathedralSmear)->load();
    target.freeze = parameters.getRawParameterValue(staticcathedral::parameters::freeze)->load();
    target.feedbackGuard = parameters.getRawParameterValue(staticcathedral::parameters::feedbackGuard)->load();
    target.width = parameters.getRawParameterValue(staticcathedral::parameters::width)->load();
    target.mix = parameters.getRawParameterValue(staticcathedral::parameters::mix)->load();
    dsp.setTargets(target);

    const int totalIn = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    for (int channel = totalIn; channel < totalOut; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    auto* left = buffer.getWritePointer(0);
    auto* right = totalOut > 1 ? buffer.getWritePointer(1) : nullptr;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float outLeft = 0.0f;
        float outRight = 0.0f;
        dsp.processFrame(left[sample], right != nullptr ? right[sample] : left[sample], outLeft, outRight);
        left[sample] = outLeft;
        if (right != nullptr)
            right[sample] = outRight;
    }
}

juce::AudioProcessorEditor* StaticCathedralAudioProcessor::createEditor()
{
    return new StaticCathedralAudioProcessorEditor(*this);
}

void StaticCathedralAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = parameters.copyState().createXml())
        copyXmlToBinary(*state, destData);
}

void StaticCathedralAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StaticCathedralAudioProcessor();
}

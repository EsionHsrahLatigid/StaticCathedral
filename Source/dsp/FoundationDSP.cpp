#include "dsp/FoundationDSP.h"

#include <cmath>

namespace staticcathedral::dsp
{
void FoundationDSP::prepare(double sampleRate, int, int channels) noexcept
{
    sampleRate_ = std::isfinite(sampleRate) && sampleRate > 1000.0 ? sampleRate : 44100.0;
    channels_ = channels < 0 ? 0 : (channels > maxChannels ? maxChannels : channels);
    reset();
}

void FoundationDSP::reset() noexcept
{
    for (auto& state : channelStates_)
    {
        for (auto& line : state.lines)
            line.fill(0.0f);
        state.writePositions.fill(0);
        state.dampedFeedback.fill(0.0f);
        for (auto& diffuser : state.inputDiffusers)
            diffuser.fill(0.0f);
        for (auto& diffuser : state.outputDiffusers)
            diffuser.fill(0.0f);
        state.inputWrite.fill(0);
        state.outputWrite.fill(0);
        state.preDelay.fill(0.0f);
        state.preDelayWrite = 0;
    }

    current_ = target_;
    feedbackGainCurrent_ = feedbackGainTarget_;
    dampingCoefficientCurrent_ = dampingCoefficientTarget_;
    modulationPhase_ = 0.0f;
}

void FoundationDSP::setTargets(const ReverbParameters& parameters) noexcept
{
    target_ = sanitizeParameters(parameters);
    const float freezeGain = lerp(0.985f, 0.9975f, target_.freeze);
    const float decayGain = 0.18f + target_.decay * 0.74f + target_.size * 0.035f;
    feedbackGainTarget_ = clamp(lerp(decayGain, freezeGain, target_.freeze), 0.02f, 0.9975f);
    dampingCoefficientTarget_ = clamp(0.08f + target_.dampingTilt * 0.70f, 0.08f, 0.78f);
}

void FoundationDSP::processFrame(float inputLeft, float inputRight, float& outputLeft, float& outputRight) noexcept
{
    smoothParameters();

    inputLeft = clamp(sanitize(inputLeft), -4.0f, 4.0f);
    inputRight = clamp(sanitize(inputRight), -4.0f, 4.0f);
    if (channels_ <= 1)
        inputRight = inputLeft;

    const float wetLeft = processChannel(inputLeft, 0, -1.0f);
    const float wetRight = processChannel(inputRight, 1, 1.0f);
    const float mid = 0.5f * (wetLeft + wetRight);
    const float side = 0.5f * (wetRight - wetLeft) * (0.25f + current_.width * 1.45f);

    const float spreadLeft = mid - side;
    const float spreadRight = mid + side;
    const float mix = current_.mix;
    outputLeft = sanitize(inputLeft + (spreadLeft - inputLeft) * mix);
    outputRight = sanitize(inputRight + (spreadRight - inputRight) * mix);
}

float FoundationDSP::processMono(float input) noexcept
{
    float left = 0.0f;
    float right = 0.0f;
    processFrame(input, input, left, right);
    return 0.5f * (left + right);
}

float FoundationDSP::processChannel(float input, int channel, float stereoSign) noexcept
{
    auto& state = channelStates_[channel <= 0 ? 0 : 1];

    const int preSamples = static_cast<int>(current_.preDelay * 0.060f * static_cast<float>(sampleRate_));
    const int boundedPreSamples = preSamples < 1 ? 1 : (preSamples >= preDelayLength ? preDelayLength - 1 : preSamples);
    const int preRead = (state.preDelayWrite - boundedPreSamples + preDelayLength) % preDelayLength;
    const float predelayed = state.preDelay[preRead];
    state.preDelay[state.preDelayWrite] = input;
    state.preDelayWrite = (state.preDelayWrite + 1) % preDelayLength;

    float diffusedInput = predelayed * (1.0f - current_.freeze * 0.95f);
    const float diffusionCoefficient = 0.15f + current_.diffusion * 0.68f;
    for (int stage = 0; stage < diffusionStages; ++stage)
        diffusedInput = processAllpass(state.inputDiffusers[stage], state.inputWrite[stage], diffusionLengths[stage], diffusionCoefficient, diffusedInput);

    std::array<float, lineCount> reads {};
    const float modulationDepthSamples = current_.modulationDepth * (2.0f + current_.cathedralSmear * 22.0f);
    for (int line = 0; line < lineCount; ++line)
    {
        const float phase = modulationPhase_ + static_cast<float>(line) * 0.727f + stereoSign * 0.31f;
        const float modulatedDelay = static_cast<float>(primeDelayLengths[line]) + std::sin(phase) * modulationDepthSamples;
        reads[line] = readFractional(state.lines[line], state.writePositions[line], clamp(modulatedDelay, 32.0f, static_cast<float>(primeDelayLengths[line] + 24)));
    }

    float wet = 0.0f;
    for (int line = 0; line < lineCount; ++line)
    {
        const float matrixed = feedbackMatrixRow(reads, line);
        state.dampedFeedback[line] += (matrixed - state.dampedFeedback[line]) * dampingCoefficientCurrent_;
        const float lineInput = diffusedInput * (line % 2 == 0 ? 0.34f : -0.34f)
            + state.dampedFeedback[line] * feedbackGainCurrent_;
        const float guarded = current_.feedbackGuard > 0.001f
            ? lerp(lineInput, softLimit(lineInput), current_.feedbackGuard)
            : lineInput;

        state.lines[line][state.writePositions[line]] = clamp(sanitize(guarded), -1.35f, 1.35f);
        state.writePositions[line] = (state.writePositions[line] + 1) % maxDelayLength;

        const float sign = ((line + (stereoSign > 0.0f ? 1 : 0)) % 3 == 0) ? -1.0f : 1.0f;
        wet += reads[line] * sign;
    }

    wet *= 0.155f + current_.size * 0.035f;
    const float smearCoefficient = 0.08f + current_.cathedralSmear * 0.72f;
    for (int stage = diffusionStages - 1; stage >= 0; --stage)
        wet = processAllpass(state.outputDiffusers[stage], state.outputWrite[stage], diffusionLengths[stage], smearCoefficient, wet);

    return clamp(sanitize(wet), -1.25f, 1.25f);
}

void FoundationDSP::smoothParameters() noexcept
{
    constexpr float smoothing = 0.0018f;
    current_.size += (target_.size - current_.size) * smoothing;
    current_.decay += (target_.decay - current_.decay) * smoothing;
    current_.preDelay += (target_.preDelay - current_.preDelay) * smoothing;
    current_.diffusion += (target_.diffusion - current_.diffusion) * smoothing;
    current_.dampingTilt += (target_.dampingTilt - current_.dampingTilt) * smoothing;
    current_.modulationDepth += (target_.modulationDepth - current_.modulationDepth) * smoothing;
    current_.modulationRate += (target_.modulationRate - current_.modulationRate) * smoothing;
    current_.cathedralSmear += (target_.cathedralSmear - current_.cathedralSmear) * smoothing;
    current_.freeze += (target_.freeze - current_.freeze) * smoothing;
    current_.feedbackGuard += (target_.feedbackGuard - current_.feedbackGuard) * smoothing;
    current_.width += (target_.width - current_.width) * smoothing;
    current_.mix += (target_.mix - current_.mix) * smoothing;
    feedbackGainCurrent_ += (feedbackGainTarget_ - feedbackGainCurrent_) * smoothing;
    dampingCoefficientCurrent_ += (dampingCoefficientTarget_ - dampingCoefficientCurrent_) * smoothing;

    modulationPhase_ += twoPi * (0.03f + current_.modulationRate * 1.95f) / static_cast<float>(sampleRate_);
    if (modulationPhase_ > twoPi)
        modulationPhase_ -= twoPi;
}

float FoundationDSP::feedbackMatrixRow(const std::array<float, lineCount>& reads, int row) const noexcept
{
    float sum = 0.0f;
    for (float value : reads)
        sum += value;
    return sum * (2.0f / static_cast<float>(lineCount)) - reads[static_cast<std::size_t>(row)];
}

float FoundationDSP::sanitize(float value) noexcept
{
    return std::isfinite(value) ? value : 0.0f;
}

float FoundationDSP::clamp(float value, float lo, float hi) noexcept
{
    return value < lo ? lo : (value > hi ? hi : value);
}

float FoundationDSP::softLimit(float value) noexcept
{
    return std::tanh(clamp(value, -8.0f, 8.0f));
}

float FoundationDSP::lerp(float a, float b, float t) noexcept
{
    return a + (b - a) * clamp(t, 0.0f, 1.0f);
}

float FoundationDSP::readFractional(const std::array<float, maxDelayLength>& buffer, int writePosition, float delaySamples) noexcept
{
    const float boundedDelay = clamp(delaySamples, 1.0f, static_cast<float>(maxDelayLength - 3));
    const int delayInt = static_cast<int>(boundedDelay);
    const float fraction = boundedDelay - static_cast<float>(delayInt);
    const int indexA = (writePosition - delayInt + maxDelayLength) % maxDelayLength;
    const int indexB = (indexA - 1 + maxDelayLength) % maxDelayLength;
    return buffer[static_cast<std::size_t>(indexA)] * (1.0f - fraction) + buffer[static_cast<std::size_t>(indexB)] * fraction;
}

float FoundationDSP::processAllpass(std::array<float, preDelayLength>& buffer, int& writePosition, int delaySamples, float coefficient, float input) noexcept
{
    const int boundedDelay = delaySamples < 1 ? 1 : (delaySamples >= preDelayLength ? preDelayLength - 1 : delaySamples);
    const int readPosition = (writePosition - boundedDelay + preDelayLength) % preDelayLength;
    const float delayed = buffer[static_cast<std::size_t>(readPosition)];
    const float output = -coefficient * input + delayed;
    buffer[static_cast<std::size_t>(writePosition)] = input + coefficient * output;
    writePosition = (writePosition + 1) % preDelayLength;
    return sanitize(output);
}

ReverbParameters FoundationDSP::sanitizeParameters(const ReverbParameters& parameters) noexcept
{
    ReverbParameters safe {};
    safe.size = clamp(sanitize(parameters.size), 0.0f, 1.0f);
    safe.decay = clamp(sanitize(parameters.decay), 0.0f, 1.0f);
    safe.preDelay = clamp(sanitize(parameters.preDelay), 0.0f, 1.0f);
    safe.diffusion = clamp(sanitize(parameters.diffusion), 0.0f, 1.0f);
    safe.dampingTilt = clamp(sanitize(parameters.dampingTilt), 0.0f, 1.0f);
    safe.modulationDepth = clamp(sanitize(parameters.modulationDepth), 0.0f, 1.0f);
    safe.modulationRate = clamp(sanitize(parameters.modulationRate), 0.0f, 1.0f);
    safe.cathedralSmear = clamp(sanitize(parameters.cathedralSmear), 0.0f, 1.0f);
    safe.freeze = clamp(sanitize(parameters.freeze), 0.0f, 1.0f);
    safe.feedbackGuard = clamp(sanitize(parameters.feedbackGuard), 0.0f, 1.0f);
    safe.width = clamp(sanitize(parameters.width), 0.0f, 1.0f);
    safe.mix = clamp(sanitize(parameters.mix), 0.0f, 1.0f);
    return safe;
}
} // namespace staticcathedral::dsp

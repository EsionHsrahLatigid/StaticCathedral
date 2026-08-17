#pragma once

#include <array>
#include <cstddef>

namespace staticcathedral::dsp
{
struct ReverbParameters
{
    float size { 0.62f };
    float decay { 0.72f };
    float preDelay { 0.18f };
    float diffusion { 0.78f };
    float dampingTilt { 0.42f };
    float modulationDepth { 0.24f };
    float modulationRate { 0.20f };
    float cathedralSmear { 0.58f };
    float freeze { 0.0f };
    float feedbackGuard { 0.72f };
    float width { 0.84f };
    float mix { 0.42f };
};

class StaticCathedralDSP
{
public:
    static constexpr int lineCount = 8;
    static constexpr std::array<int, lineCount> primeDelayLengths { 1151, 1427, 1699, 1999, 2371, 2749, 3167, 3571 };

    void prepare(double sampleRate, int maxBlockSize, int channels) noexcept;
    void reset() noexcept;
    void setTargets(const ReverbParameters& parameters) noexcept;
    void processFrame(float inputLeft, float inputRight, float& outputLeft, float& outputRight) noexcept;
    float processMono(float input) noexcept;

    int preparedChannels() const noexcept { return channels_; }
    double sampleRate() const noexcept { return sampleRate_; }
    float currentFeedbackGain() const noexcept { return feedbackGainCurrent_; }

private:
    static constexpr int maxChannels = 2;
    static constexpr int maxDelayLength = 4096;
    static constexpr int preDelayLength = 4096;
    static constexpr int diffusionStages = 4;
    static constexpr std::array<int, diffusionStages> diffusionLengths { 67, 149, 293, 521 };
    static constexpr float twoPi = 6.2831853071795864769f;

    struct ChannelState
    {
        std::array<std::array<float, maxDelayLength>, lineCount> lines {};
        std::array<int, lineCount> writePositions {};
        std::array<float, lineCount> dampedFeedback {};
        std::array<std::array<float, preDelayLength>, diffusionStages> inputDiffusers {};
        std::array<std::array<float, preDelayLength>, diffusionStages> outputDiffusers {};
        std::array<int, diffusionStages> inputWrite {};
        std::array<int, diffusionStages> outputWrite {};
        std::array<float, preDelayLength> preDelay {};
        int preDelayWrite { 0 };
    };

    static float sanitize(float value) noexcept;
    static float clamp(float value, float lo, float hi) noexcept;
    static float softLimit(float value) noexcept;
    static float lerp(float a, float b, float t) noexcept;
    static float readFractional(const std::array<float, maxDelayLength>& buffer, int writePosition, float delaySamples) noexcept;
    static float processAllpass(std::array<float, preDelayLength>& buffer, int& writePosition, int delaySamples, float coefficient, float input) noexcept;
    static ReverbParameters sanitizeParameters(const ReverbParameters& parameters) noexcept;

    float processChannel(float input, int channel, float stereoSign) noexcept;
    void smoothParameters() noexcept;
    float feedbackMatrixRow(const std::array<float, lineCount>& reads, int row) const noexcept;

    double sampleRate_ { 44100.0 };
    int channels_ { 0 };
    std::array<ChannelState, maxChannels> channelStates_ {};

    ReverbParameters current_ {};
    ReverbParameters target_ {};
    float feedbackGainCurrent_ { 0.68f };
    float feedbackGainTarget_ { 0.68f };
    float dampingCoefficientCurrent_ { 0.35f };
    float dampingCoefficientTarget_ { 0.35f };
    float modulationPhase_ { 0.0f };
};
} // namespace staticcathedral::dsp

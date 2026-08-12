#include "TestSupport.h"
#include "dsp/FoundationDSP.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

namespace
{
bool isPrime(int value)
{
    if (value < 2)
        return false;
    for (int divisor = 2; divisor * divisor <= value; ++divisor)
        if (value % divisor == 0)
            return false;
    return true;
}

staticcathedral::dsp::ReverbParameters wetParams()
{
    staticcathedral::dsp::ReverbParameters params;
    params.size = 0.70f;
    params.decay = 0.46f;
    params.preDelay = 0.0f;
    params.diffusion = 0.86f;
    params.dampingTilt = 0.46f;
    params.modulationDepth = 0.0f;
    params.modulationRate = 0.0f;
    params.cathedralSmear = 0.68f;
    params.freeze = 0.0f;
    params.feedbackGuard = 0.75f;
    params.width = 0.80f;
    params.mix = 1.0f;
    return params;
}

std::vector<float> renderImpulse(const staticcathedral::dsp::ReverbParameters& params, int samples)
{
    auto dsp = std::make_unique<staticcathedral::dsp::FoundationDSP>();
    dsp->setTargets(params);
    dsp->prepare(48000.0, 512, 2);
    std::vector<float> output(static_cast<std::size_t>(samples));
    for (int i = 0; i < samples; ++i)
    {
        float left = 0.0f;
        float right = 0.0f;
        dsp->processFrame(i == 0 ? 1.0f : 0.0f, 0.0f, left, right);
        output[static_cast<std::size_t>(i)] = 0.5f * (left + right);
    }
    return output;
}

float energy(const std::vector<float>& values, int begin, int end)
{
    float total = 0.0f;
    for (int i = begin; i < end; ++i)
        total += values[static_cast<std::size_t>(i)] * values[static_cast<std::size_t>(i)];
    return total;
}

float absoluteDifference(const std::vector<float>& values, int begin, int end)
{
    float total = 0.0f;
    for (int i = begin + 1; i < end; ++i)
        total += std::abs(values[static_cast<std::size_t>(i)] - values[static_cast<std::size_t>(i - 1)]);
    return total;
}
} // namespace

int main()
{
    return test_support::run("staticcathedral_dsp_tests", [] {
        for (const auto length : staticcathedral::dsp::FoundationDSP::primeDelayLengths)
            test_support::check(isPrime(length), "all FDN delay lengths are prime");

        auto lengths = staticcathedral::dsp::FoundationDSP::primeDelayLengths;
        std::sort(lengths.begin(), lengths.end());
        test_support::check(std::adjacent_find(lengths.begin(), lengths.end()) == lengths.end(), "prime delay lengths are unique");

        auto params = wetParams();
        const auto impulse = renderImpulse(params, 160000);
        test_support::check(energy(impulse, 1500, 16000) > 0.0001f, "input/output diffusion creates a dense early response");
        test_support::check(energy(impulse, 120000, 155000) < energy(impulse, 48000, 83000), "non-freeze impulse tail decays");

        const auto impulseRepeat = renderImpulse(params, 160000);
        float deterministicError = 0.0f;
        for (std::size_t i = 0; i < impulse.size(); ++i)
            deterministicError += std::abs(impulse[i] - impulseRepeat[i]);
        test_support::check(deterministicError == 0.0f, "modulation-free render is sample deterministic");

        auto dark = params;
        dark.dampingTilt = 0.02f;
        auto bright = params;
        bright.dampingTilt = 0.98f;
        const auto darkImpulse = renderImpulse(dark, 36000);
        const auto brightImpulse = renderImpulse(bright, 36000);
        test_support::check(absoluteDifference(brightImpulse, 7000, 26000) > absoluteDifference(darkImpulse, 7000, 26000),
            "damping tilt changes high-frequency decay texture");

        auto frozen = params;
        frozen.decay = 1.0f;
        frozen.freeze = 1.0f;
        frozen.feedbackGuard = 1.0f;
        auto freezeDsp = std::make_unique<staticcathedral::dsp::FoundationDSP>();
        freezeDsp->setTargets(frozen);
        freezeDsp->prepare(48000.0, 256, 2);
        float previousWindow = 0.0f;
        float currentWindow = 0.0f;
        for (int i = 0; i < 96000; ++i)
        {
            float left = 0.0f;
            float right = 0.0f;
            freezeDsp->processFrame(i == 0 ? 1.0f : 0.0f, i == 0 ? -1.0f : 0.0f, left, right);
            test_support::check(std::isfinite(left) && std::isfinite(right), "freeze output finite");
            const float sampleEnergy = left * left + right * right;
            if (i >= 48000 && i < 60000)
                previousWindow += sampleEnergy;
            if (i >= 84000)
                currentWindow += sampleEnergy;
            test_support::check(std::abs(left) <= 1.5f && std::abs(right) <= 1.5f, "freeze safety guard bounds output");
        }
        test_support::check(currentWindow <= previousWindow * 1.12f + 0.0001f, "freeze remains bounded rather than growing");
        test_support::check(freezeDsp->currentFeedbackGain() < 1.0f, "feedback spectral radius kept below unity");

        auto finiteDsp = std::make_unique<staticcathedral::dsp::FoundationDSP>();
        finiteDsp->setTargets(params);
        finiteDsp->prepare(44100.0, 0, 1);
        for (int i = 0; i < 128; ++i)
            test_support::check(std::isfinite(finiteDsp->processMono(i == 0 ? std::numeric_limits<float>::infinity() : 0.0f)),
                "non-finite mono input sanitized");
        finiteDsp->reset();
        const auto resetA = finiteDsp->processMono(0.25f);
        finiteDsp->reset();
        const auto resetB = finiteDsp->processMono(0.25f);
        test_support::check(resetA == resetB, "reset restores deterministic state");

        auto stereoDsp = std::make_unique<staticcathedral::dsp::FoundationDSP>();
        stereoDsp->setTargets(params);
        stereoDsp->prepare(48000.0, 128, 2);
        float left = 0.0f;
        float right = 0.0f;
        for (int i = 0; i < 4096; ++i)
            stereoDsp->processFrame(i == 0 ? 1.0f : 0.0f, 0.0f, left, right);
        test_support::check(std::isfinite(left) && std::isfinite(right), "stereo processing finite");
        test_support::check(std::abs(left - right) > 0.000001f, "stereo width creates deterministic channel contrast");
    });
}

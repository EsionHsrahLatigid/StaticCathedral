#pragma once

#include <array>
#include <cstddef>

namespace staticcathedral::parameters
{
inline constexpr const char* size = "size";
inline constexpr const char* decay = "decay";
inline constexpr const char* preDelay = "preDelay";
inline constexpr const char* diffusion = "diffusion";
inline constexpr const char* dampingTilt = "dampingTilt";
inline constexpr const char* modulationDepth = "modulationDepth";
inline constexpr const char* modulationRate = "modulationRate";
inline constexpr const char* cathedralSmear = "cathedralSmear";
inline constexpr const char* freeze = "freeze";
inline constexpr const char* feedbackGuard = "feedbackGuard";
inline constexpr const char* width = "width";
inline constexpr const char* mix = "mix";

inline constexpr std::array<const char*, 12> all {
    size,
    decay,
    preDelay,
    diffusion,
    dampingTilt,
    modulationDepth,
    modulationRate,
    cathedralSmear,
    freeze,
    feedbackGuard,
    width,
    mix,
};

inline constexpr std::array<const char*, 12> names {
    "Size",
    "Decay",
    "Pre-delay",
    "Diffusion",
    "Damping tilt",
    "Mod depth",
    "Mod rate",
    "Cathedral smear",
    "Freeze",
    "Feedback guard",
    "Width",
    "Mix",
};

inline constexpr std::array<const char*, 12> tooltips {
    "Scales the apparent cavity and late-field density without changing the fixed prime delay lengths.",
    "Maps the FDN feedback gain below unity for deterministic decays.",
    "Adds bounded input pre-delay before the cathedral network.",
    "Controls input and output allpass diffusion depth.",
    "Tilts feedback damping from dark absorption to brighter metallic decay.",
    "Controls bounded fractional modulation of each delay line.",
    "Controls deterministic delay modulation speed.",
    "Adds hostile late-field smearing and output diffusion.",
    "Crossfades into a bounded near-hold state with reduced input injection.",
    "Enables the saturating safety guard inside the feedback loop.",
    "Spreads the wet image using deterministic left/right line polarities.",
    "Blends dry input with the StaticCathedral wet field.",
};
} // namespace staticcathedral::parameters

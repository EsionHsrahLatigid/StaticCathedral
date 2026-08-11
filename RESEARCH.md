# Research

StaticCathedral maps the DHN9 reverb decision record to a bounded JUCE implementation.

## Sources

- M. R. Schroeder, "Natural Sounding Artificial Reverberation", AES e-library record `id=849`: source for comb/allpass artificial reverberation and diffusion as a reverb-building method.
- J.-M. Jot and A. Chaigne, AES e-library record `id=5663`: source for time-varying reverberation and FDN context.
- Schlecht and Habets, "On Lossless Feedback Delay Networks", arXiv `1606.07729`: source for orthogonal/lossless FDN matrix reasoning and stability framing.
- JUCE 8.0.15 commit `91ad83ae34a81e0833b1a2b0866f54846370ae53`: source for the plugin framework, APVTS, CMake integration, and editor widgets.
- Local Plitch evidence from DHN9 G001/G002: source for EHL identity, CMake/JUCE shape, independent DSP tests, plugin integration tests, and artifact staging.

## Mapping

- Schroeder diffusion -> input and output allpass stages around the late reverberator.
- Jot/Chaigne FDN reverberation -> time-varying multi-line recursive late field.
- Schlecht/Habets lossless FDN reasoning -> Householder orthogonal feedback matrix before damping and safety guard.
- DHN interpretation -> fixed hostile prime lengths, high-density metallic texture, modulation, freeze, and hard finite bounds.
- JUCE/APVTS -> automatable parameters, state round-trip, custom accessible editor controls.

## Honest Limits

This implementation is algorithmic, not convolutional, and does not model a physical cathedral. The orthogonal matrix is energy-preserving only before damping, feedback gain, freeze, saturation, and output scaling. The fixed prime delays are exact at every sample rate, so their time durations vary with sample rate. Windows artifact staging is encoded and checked by CMake but was not locally built on macOS.

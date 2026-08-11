# Design

## DSP

The reverb core is `staticcathedral::dsp::FoundationDSP`, retained as a generated target name but implemented as the StaticCathedral engine.

- Eight fixed prime delay lengths: `1151`, `1427`, `1699`, `1999`, `2371`, `2749`, `3167`, `3571` samples.
- Feedback matrix: Householder orthogonal matrix, energy-preserving before damping, gain, freeze, and guard stages.
- Diffusion: four small prime-ish allpass stages before the FDN and four reversed stages after it.
- Damping: one-pole feedback smoothing controlled by `Damping tilt`, giving a dark-to-bright decay contrast.
- Modulation: deterministic sinusoidal fractional-read offsets bounded to a small sample range.
- Freeze: near-hold feedback remains below unity, attenuates new input, and is guarded by saturation.
- Safety: no heap allocation, locks, I/O, logging, exceptions, or unbounded loops in processing; all input and output samples are finite-sanitized and bounded.

The delay lengths are fixed rather than sample-rate scaled so the prime-length and deterministic test contracts remain exact. `Size` changes apparent space, gain, and density rather than reallocating delay memory.

## UI

The editor follows the DHN9 shared visual contract: 960 x 544 default, 720 x 432 minimum, 8 px grid, grayscale only, no image assets, no external fonts, and no `GenericAudioProcessorEditor`.

Every parameter has a visible slider with:

- stable component ID `staticcathedral-control-<parameter-id>`;
- accessible name/title/description;
- tooltip text matching the parameter role;
- keyboard focus.

The motif is a procedural grayscale 8-bit cathedral: block arches driven by Size/Decay/Diffusion and a bottom decay skyline driven by Smear/Freeze. It is intentionally simple and inspectable rather than asset-based.

# StaticCathedral

StaticCathedral is an EsionHsrahLatigid Digital Harsh Noise reverb plugin. It is intentionally metallic and hostile: an eight-line prime-length feedback delay network with input/output diffusion, frequency-dependent damping, deterministic delay modulation, bounded freeze, and a saturating feedback guard.

## Identity

- Product: `StaticCathedral`
- Repository slug: `staticcathedral`
- Bundle ID: `jp.ehl.staticcathedral`
- Manufacturer: `EsionHsrahLatigid`
- Manufacturer code: `EHL_`
- Plugin code: `StCt`
- Formats: VST3 and Standalone everywhere JUCE supports them; AU on Apple

## Parameters

All parameters are exposed through APVTS, host automation, and named/tooltipped custom editor controls:

`Size`, `Decay`, `Pre-delay`, `Diffusion`, `Damping tilt`, `Mod depth`, `Mod rate`, `Cathedral smear`, `Freeze`, `Feedback guard`, `Width`, `Mix`.

## Build

Use an existing JUCE checkout for offline/local builds:

```sh
cmake --preset engine-debug
cmake --build --preset engine-debug
ctest --preset engine-debug --output-on-failure

cmake --preset plugin-release -DEHL_JUCE_SOURCE_DIR=/path/to/JUCE
cmake --build --preset plugin-release --parallel 2
ctest --preset plugin-release --output-on-failure
```

If `EHL_JUCE_SOURCE_DIR` is not provided, CMake FetchContent uses JUCE commit `91ad83ae34a81e0833b1a2b0866f54846370ae53`, matching the DHN9/Plitch baseline.

Stable staged artifacts:

```text
artifacts/plugin-release/macos-arm64/standalone/staticcathedral_standalone_plugin.app
artifacts/plugin-release/macos-arm64/vst3/staticcathedral_vst3_plugin.vst3
artifacts/plugin-release/macos-arm64/au/staticcathedral_au_plugin.component
artifacts/plugin-release/macos-arm64/ARTIFACTS.txt

artifacts/plugin-release/windows-x64/standalone/staticcathedral_standalone_plugin.exe
artifacts/plugin-release/windows-x64/vst3/staticcathedral_vst3_plugin.vst3
artifacts/plugin-release/windows-x64/ARTIFACTS.txt
```

## Tests

- `staticcathedral_dsp_tests`: prime delay structure, early density, deterministic decay, damping contrast, bounded freeze, finite/reset behavior, mono/stereo processing.
- `staticcathedral_plugin_tests`: metadata, matched mono/stereo bus layouts, state round-trip, invalid-state tolerance, finite audio processing.
- `staticcathedral_editor_tests`: custom editor contract, no generic editor, accessible named/tooltipped controls for every parameter, and the shared EHL module header/divider/plain-body contract.
- `staticcathedral_artifact_contract`: staged product paths and bundle shape.

## Limits

StaticCathedral is bounded and finite, but it is a harsh-noise effect, not a hearing-safety device. The feedback matrix is orthogonal before damping/guard stages and effective feedback gain is kept below unity; output is still capable of aggressive full-scale texture by design.

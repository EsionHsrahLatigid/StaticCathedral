# Build Notes

Requirements: CMake 3.22+, Ninja, a C++17 compiler, and JUCE `91ad83ae34a81e0833b1a2b0866f54846370ae53` or a compatible checkout supplied with `EHL_JUCE_SOURCE_DIR`.

## Engine Tests

```sh
cmake --preset engine-debug
cmake --build --preset engine-debug --parallel 2
ctest --preset engine-debug --output-on-failure
```

This config builds only the JUCE-independent DSP test target.

## Plugin Release

```sh
cmake --preset plugin-release -DEHL_JUCE_SOURCE_DIR=<path-to-juce-source>
cmake --build --preset plugin-release --parallel 2
ctest --preset plugin-release --output-on-failure
```

`plugin-release` builds VST3 and Standalone on every platform, AU on Apple, and stages products through `ehl_stage_products`.

## Artifact Contract

The local macOS contract is:

```text
artifacts/plugin-release/macos-arm64/vst3/staticcathedral_vst3_plugin.vst3
artifacts/plugin-release/macos-arm64/standalone/staticcathedral_standalone_plugin.app
artifacts/plugin-release/macos-arm64/au/staticcathedral_au_plugin.component
artifacts/plugin-release/macos-arm64/ARTIFACTS.txt
```

The Windows contract stages VST3 as a directory bundle, not a flat file:

```text
artifacts/plugin-release/windows-x64/vst3/staticcathedral_vst3_plugin.vst3
artifacts/plugin-release/windows-x64/standalone/staticcathedral_standalone_plugin.exe
artifacts/plugin-release/windows-x64/ARTIFACTS.txt
```

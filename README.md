# Vox Harmonic Rider (independent recreation)

This is a new JUCE 8 VST3 project inspired by the publicly observable controls and terminology in the supplied VocRide 2 binary. It is not the original source code and does not contain copied proprietary code, assets, presets, or licensing logic.

## Build on Windows

Requirements:
- Visual Studio 2022 or 2026 with Desktop development with C++
- CMake 3.22+
- Internet access during the first CMake configure (JUCE 8.0.12 is fetched automatically)

From a Developer PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release --target VoxHarmonicRider_VST3
```

To use an existing JUCE checkout instead, pass `-DJUCE_DIR=C:/JUCE` to the configure command.

The VST3 bundle will be under:
`build/VoxHarmonicRider_artefacts/Release/VST3/`

Copy the resulting `.vst3` bundle to:
`C:\Program Files\Common Files\VST3\`

## Current implementation / limitations

- Stereo and mono VST3/Standalone target.
- APVTS parameters for input, low cut, fundamental/2nd/3rd/Nth harmonic levels, Nth selector, output, dynamics threshold/strength, Solo and per-band Dynamics toggles.
- Simple independent harmonic-band processing prototype with a rudimentary level-following pitch estimate and soft output limiting.
- This is a prototype, not a bit-for-bit recreation. It does not reproduce the original VocRide pitch tracker, linear-phase FIR, latency behavior, preset compatibility, or exact UI.
- The Live Mode control is currently a placeholder; linear-phase mode is not implemented.
- Do not install over the original plugin. Use a separate plugin name and bundle ID.

The next engineering step is to replace the simple pitch estimate and filter bank with a proper pitch tracker and calibrated harmonic-band analysis, then test against reference audio.

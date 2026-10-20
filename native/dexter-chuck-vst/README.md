# Dexter U.S. Chuck VST

An independent ChucK-based VST3 project with two plug-ins:

- **Dexter U.S. Chuck Instrument**: runs a ChucK script as a stereo instrument.
- **Dexter U.S. Chuck Effect**: routes stereo DAW audio through a ChucK script.

Both share a syntax-highlighted ChucK editor with Open, Save, Run, Stop, and a
built-in example. The instrument and effect do not expose MIDI ports or host
automation. ChucK's own sample-time scheduling and shreds remain available.
The editor buffer is saved with DAW project state; restored code is not
automatically executed.

The VST3 face identifies the maker as **Dexter U.S.** and displays
**Dexter U.S. Chuck**. Example scripts are kept in separate `Examples/Instruments`
and `Examples/Effects` folders and are copied into the VST3 bundle's
`Contents/Resources` directory.

## Build

Requires CMake 3.22+, a C++17 compiler, Git, and network access on first build.
JUCE 9.0.4 and ChucK 1.5.5.8 are pinned in the CMake project.

```sh
cmake -S native/dexter-chuck-vst -B native/dexter-chuck-vst/build \
  -DCMAKE_BUILD_TYPE=Release
cmake --build native/dexter-chuck-vst/build --parallel 2
ctest --test-dir native/dexter-chuck-vst/build --output-on-failure
```

On Windows, configure with Visual Studio 2022 x64 and build both VST3 targets:

```powershell
cmake -S native/dexter-chuck-vst -B native/dexter-chuck-vst/build-windows `
  -G "Visual Studio 17 2022" -A x64
cmake --build native/dexter-chuck-vst/build-windows --config Release `
  --target DexterChuckInstrument_VST3 DexterChuckEffect_VST3 --parallel 4
```

## License and commercial use

This project is licensed under **AGPL-3.0-only**; see `LICENSE`. The AGPL
permits charging for copies and services. Recipients must retain their AGPL
rights, and the license requires source availability in the situations it
covers, including certain network use. It does not provide exclusive control
over downstream redistribution.

The project selects ChucK's MIT license option. JUCE 9.0.4 is used under its
AGPLv3 option. Third-party license notices and the pinned JUCE software bill of
materials are in `THIRD_PARTY_NOTICES.md` and `third_party/licenses/`.
This project does not include Chuck Racks application code or Sonic Pi VST
source; the editor is an independent implementation.

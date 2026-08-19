[README-3.md](https://github.com/user-attachments/files/31232970/README-3.md)
# Concert Venue Simulator (JUCE / C++)

A real-time audio plugin that simulates the acoustic character of different concert venues — Club, Theatre, and Arena — built as the final project for the Music and Audio Programming module (ECS7012P) at Queen Mary University of London.

## Overview

Dry audio signals lack the spatial qualities of a real performance space. This plugin recreates that character by combining reverb, low-pass filtering, and stereo width processing into a single signal chain, with three venue presets grounded in real acoustic principles.

## Signal chain

Applied in sequence inside `processBlock()`:

1. **Stereo width (Mid/Side processing)** — encodes L/R into Mid + Side components, scales the Side channel by the `width` parameter to control spatial spread
2. **Low-pass filter (IIR)** — models air absorption of high frequencies over distance; cutoff drops as venue size increases
3. **Reverb (Schroeder algorithm, via `juce::Reverb`)** — simulates room reflections and decay
4. **Wet/dry mix** — linear blend of processed and original signal

The order matters: filtering is applied before reverb so the reverb tail inherits the darker frequency content — filtering afterward would reintroduce bright high frequencies into the tail. Width processing comes first so the reverb acts on an already-spread signal.

## Presets

Grounded in Sabine's reverberation equation, RT60 = 0.161 × V/A (room volume over total absorption), which maps room size and absorption to decay time:

| Parameter | Club | Theatre | Arena |
|---|---|---|---|
| Room Size | 0.20 | 0.55 | 0.75 |
| Damping | 0.70 | 0.50 | 0.40 |
| Wet Level | 0.30 | 0.50 | 0.80 |
| Dry Level | 0.70 | 0.50 | 0.60 |
| Width | 0.40 | 0.70 | 0.85 |
| LPF Cutoff | ~8000 Hz | ~5000 Hz | ~3500 Hz |
| Approx. RT60 | 0.3–0.5s | 1.2–1.8s | 2.5–3.5s |

## Validation

Two test signals were run through all three presets and compared via spectrogram:

- **White noise** — its flat spectrum makes the effect of each preset unambiguous: LPF cutoff visibly drops and the reverb tail lengthens from Club → Theatre → Arena.
- **Music sample** — confirms the same pattern audibly and visually: tight and dry in Club, wide and spacious in Arena, with the reverb tail showing as increased spectral density trailing each transient.

Both recordings are included in [`Audio Samples/`](./Audio%20Samples).

See [`Report.pdf`](./Report.pdf) for the full writeup and [`Presentation.pptx`](./Presentation.pptx) for the project slides.

## Project structure

```
ConcertVenueSimulator/
├── Source/
│   ├── PluginProcessor.h/.cpp   # DSP: width, filter, reverb, file/white noise handling
│   └── PluginEditor.h/.cpp      # UI: preset buttons, six parameter knobs, file loader
├── Audio Samples/
│   ├── Whitenoise_ClubTheatreArena.wav
│   └── Music_ClubTheatreArena.wav
├── Report.pdf
└── Presentation.pptx
```

## Key learnings

- Real-time audio processing tolerates no memory allocation or file I/O inside `processBlock()` — everything must be pre-allocated to meet the buffer deadline.
- DSP stage order changes the result fundamentally — the reverb tail inherits whatever frequency content precedes it.
- Sabine's equation gave the venue presets a physical basis rather than tuning by ear alone.
- `AudioProcessorValueTreeState` binds parameters to the GUI automatically, cutting out a lot of manual glue code.

## Building

1. Open the project in [Projucer](https://juce.com/get-juce/) (part of the JUCE framework) — note: the `.jucer` project file isn't included in this repo; recreate a JUCE Audio Plugin project and add the files in `Source/`.
2. Export to your IDE of choice (Xcode, Visual Studio, etc.) and build.
3. Requires the [JUCE](https://juce.com/) framework.

## Author

Sanjay Yamasandhi Sundresh — MSc Advanced Electrical and Electronics Engineering, QMUL

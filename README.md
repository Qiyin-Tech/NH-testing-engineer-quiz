# Qiyin Tech Testing Engineer Quiz
This project is a simple audio player (approximately 600 lines of C++) built with the JUCE framework. It supports real-time audio effect processing.

## Rule
In this quiz, you are required to:
1. **Build and run the project in C++.**
Use the provided `test_audio.mp3` to test the program. Compare the playback quality with your system’s media player, and identify as many bugs as possible across different modules.
Please describe your findings in a report, **organized by module**.
> Note: The project contains **at least five bugs**, including one that is **specific to Apple platforms**.

2. **iOS Mini App Development**
Fork the repository, fix any **platform-specific compilation issues**, and implement a minimal iOS app capable of **decoding and playing audio**.
The effects module is optional and may be omitted — only basic playback is required.

## Modules
```
├── Player Module (player.h/cpp)
│   ├── PlayerProcessor Class
│   │   ├── Audio Device Management
│   │   │   ├── initializeAudio() - Initialize audio device with sample rate and buffer size
│   │   │   ├── shutdownAudio() - Clean up audio device resources
│   │   │   └── AudioCallback inner class - Handles JUCE audio device callbacks
│   │   ├── Playback Control
│   │   │   ├── play() - Start/resume audio playback
│   │   │   ├── pause() - Pause audio playback
│   │   │   ├── stop() - Stop playback and reset position
│   │   │   └── seek() - Move playback cursor to specific sample position
│   │   ├── File Management
│   │   │   └── load() - Load audio files using JUCE AudioFormatManager
│   │   ├── Effects Chain Management
│   │   │   ├── addEffect() - Add effect processor to chain
│   │   │   ├── removeEffect() - Remove effect from chain
│   │   │   ├── clearEffects() - Clear all effects
│   │   │   └── resetAllEffects() - Reset all effect states
│   │   └── Audio Processing
│   │       ├── audioDeviceIOCallback() - Main audio processing callback
│   │       ├── processingBlock() - Apply effects to audio buffer
│   │       └── processBuffer - Temporary buffer for audio processing
│   └── JUCE Dependencies
│       ├── juce_audio_basics - Basic audio functionality
│       ├── juce_audio_devices - Audio device management
│       └── juce_audio_formats - Audio file format support
│
├── Effects Module (effect.h/cpp)
│   ├── EffectProcessor Interface (Abstract Base Class)
│   │   ├── processingBlock() - Pure virtual audio processing method
│   │   ├── resetState() - Pure virtual effect state reset
│   │   └── setSampleRate() - Pure virtual sample rate configuration
│   └── BiquadFilter Implementation
│       ├── Filter Types
│       │   ├── LowPass - Low-pass filter
│       │   ├── HighPass - High-pass filter
│       │   ├── BandPass - Band-pass filter
│       │   └── Notch - Notch filter
│       ├── Filter Parameters
│       │   ├── setFrequency() - Set cutoff frequency
│       │   ├── setQ() - Set resonance/Q factor
│       │   └── setGain() - Set gain
│       ├── Biquad Processing
│       │   ├── calculateCoefficients() - Calculate filter coefficients
│       │   ├── State Variables (x1, x2, y1, y2) - Per-channel filter history
│       │   └── Coefficients (b0, b1, b2, a0, a1, a2) - Filter coefficients
│       └── Channel Processing
│           └── Multi-channel support with independent state per channel
│
└── Main Application (main.cpp)
    ├── Command-line Interface
    │   ├── Argument parsing for audio file path
    │   └── Error handling and user feedback
    ├── Audio Player Setup
    │   ├── PlayerProcessor initialization
    │   ├── Audio device configuration (44.1kHz, 1024 samples)
    │   └── High-pass filter setup (120Hz cutoff)
    ├── Playback Control
    │   ├── Audio file loading
    │   ├── Playback start/stop
    │   └── User input handling (Enter key to stop)
    └── Status Monitoring
        ├── Background monitoring thread
        ├── Playback status display
        └── Position tracking and completion detection
```

## Processing Procedure in `main.cpp`
```
Audio Processing Flow (Real-time):
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│ Audio File      │     │ Effects Chain    │     │ Audio Device    │
│ Reader          │───▶│ (BiquadFilter)   │───▶│ Output          │
└─────────────────┘     └──────────────────┘     └─────────────────┘
       │                        │                        │
       ▼                        ▼                        ▼
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│ Read Samples    │     │ Apply Filter     │     │ Play Audio      │
│ from File       │     │ Processing       │     │ through Device  │
└─────────────────┘     └──────────────────┘     └─────────────────┘
```

## Development
### Building
```
git clone --recursive https://github.com/Qiyin-Tech/NH-testing-engineer-quiz.git
cd NH-testing-engineer-quiz
mkdir build
cd build
cmake ..
make
```

### Run
The main executable is `AudioPlayer_main`.
```
AudioPlayer_main <path_to_audio_file>
```


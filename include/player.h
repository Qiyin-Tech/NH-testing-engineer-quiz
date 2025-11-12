#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include "effect.h"

class PlayerProcessor {
public:
    PlayerProcessor();  // Initialize audio device
    ~PlayerProcessor();

    bool play();  // Continue playing
    uint64_t seek(uint64_t sample);  // move the playing cursor
    bool pause();  // Pause playing

    bool load(const std::string& filePath);  // Load a audio file
    bool stop();  // Pause and move playing cursor to the start

    void processingBlock(juce::AudioBuffer<float>& buffer);  // Slot for effects
    
    // Effect management
    void addEffect(std::shared_ptr<EffectProcessor> effect);
    void removeEffect(std::shared_ptr<EffectProcessor> effect);
    void clearEffects();
    void resetAllEffects();
    
    // Audio device management
    bool initializeAudio(double sampleRate = 44100.0, int bufferSize = 512);
    void shutdownAudio();
    
    // Getters
    bool isPlaying() const { return isPlayingState; }
    uint64_t getCurrentPosition() const { return currentPosition; }
    uint64_t getTotalLength() const { return totalLength; }
    double getSampleRate() const { return currentSampleRate; }

private:
    class AudioCallback : public juce::AudioIODeviceCallback {
    public:
        AudioCallback(PlayerProcessor& p) : player(p) {};
        
        void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                               int numInputChannels,
                                               float* const* outputChannelData,
                                               int numOutputChannels,
                                               int numSamples,
                                               const juce::AudioIODeviceCallbackContext& context) override {
            player.audioDeviceIOCallback(const_cast<const float**>(inputChannelData), numInputChannels,
                                    const_cast<float**>(outputChannelData), numOutputChannels, numSamples);
        }
        
        void audioDeviceAboutToStart(juce::AudioIODevice* device) {
            player.audioDeviceAboutToStart(device);
        }
        
        void audioDeviceStopped() {
            player.audioDeviceStopped();
        }
        
    private:
        PlayerProcessor& player;
    };
    
    // Audio callback methods
    void audioDeviceIOCallback(const float* const* inputChannelData, int numInputChannels,
                              float* const* outputChannelData, int numOutputChannels, int numSamples);
    void audioDeviceAboutToStart(juce::AudioIODevice* device);
    void audioDeviceStopped();
    
    void prepareToPlay(int samplesPerBlock, double sampleRate);
    void releaseResources();
    
    std::unique_ptr<AudioCallback> audioCallback;
    
    // Audio format manager and reader
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReader> reader;
    
    // Audio device
    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
    
    // Playback state
    bool isPlayingState = false;
    uint64_t currentPosition = 0;
    uint64_t totalLength = 0;
    double currentSampleRate = 44100.0;
    int currentBufferSize = 1024;
    
    // Effects chain
    std::vector<std::shared_ptr<EffectProcessor>> effects;
    
    // Temporary buffer for processing
    juce::AudioBuffer<float> processBuffer;
};

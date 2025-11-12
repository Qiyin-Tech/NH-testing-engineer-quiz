#include "player.h"
#include <thread>
#include <chrono>

PlayerProcessor::PlayerProcessor() {
    formatManager.registerBasicFormats();

    deviceManager = std::make_unique<juce::AudioDeviceManager>();
    audioCallback = std::make_unique<AudioCallback>(*this);
}

PlayerProcessor::~PlayerProcessor() {
    shutdownAudio();
}

bool PlayerProcessor::initializeAudio(double sampleRate, int bufferSize) {
    currentSampleRate = sampleRate;
    currentBufferSize = bufferSize;

    juce::AudioDeviceManager::AudioDeviceSetup setup;
    setup.sampleRate = sampleRate;
    setup.bufferSize = bufferSize;
    setup.inputChannels = 0;
    setup.outputChannels = 2; // Force to use stereo
    setup.useDefaultInputChannels = false;
    setup.useDefaultOutputChannels = true;

    auto result = deviceManager->initialise(setup.inputChannels.getHighestBit() + 1,
                                           setup.outputChannels.getHighestBit() + 1,
                                           nullptr, true, "", &setup);

    if (result.isEmpty()) {
        auto* device = deviceManager->getCurrentAudioDevice();
        if (device != nullptr) {
            std::cout << "Audio device initialized: " << device->getName().toStdString()
                      << " (sample rate: " << device->getCurrentSampleRate()
                      << ", buffer size: " << device->getCurrentBufferSizeSamples()
                      << ", output channels: " << device->getOutputChannelNames().size() << ")" << std::endl;

            deviceManager->addAudioCallback(audioCallback.get());
        } else {
            std::cerr << "Audio device initialization failed: No device available" << std::endl;
            return false;
        }

        return true;
    } else {
        std::cerr << "Audio device initialization failed: " << result.toStdString() << std::endl;
    }

    return false;
}

void PlayerProcessor::shutdownAudio() {
    if (deviceManager) {
        deviceManager->removeAudioCallback(audioCallback.get());
        // deviceManager->closeAudioDevice();
    }
}

bool PlayerProcessor::load(const std::string& filePath) {
    juce::File file(filePath);

    if (!file.existsAsFile()) {
        std::cerr << "File does not exist: " << filePath << std::endl;
        return false;
    }

    reader.reset(formatManager.createReaderFor(file));

    if (reader != nullptr) {
        totalLength = reader->lengthInSamples;
        currentPosition = 0;

        // Prepare processing buffer
        processBuffer.setSize(reader->numChannels, currentBufferSize);

        // Reset all effects when loading new file
        resetAllEffects();

        std::cout << "Successfully loaded file: " << filePath
                  << " (channels: " << reader->numChannels
                  << ", sample rate: " << reader->sampleRate
                  << ", length: " << totalLength << " samples)" << std::endl;

        return true;
    } else {
        std::cerr << "Failed to create reader for file: " << filePath
                  << ". Supported formats may not include this file type." << std::endl;

        std::cout << "Supported formats:" << std::endl;
        for (int i = 0; i < formatManager.getNumKnownFormats(); ++i) {
            auto* format = formatManager.getKnownFormat(i);
            std::cout << " - " << format->getFormatName().toStdString() << std::endl;
        }
    }

    return false;
}

bool PlayerProcessor::play() {
    if (reader != nullptr && !isPlayingState) {
        isPlayingState = true;
        return true;
    }
    return false;
}

bool PlayerProcessor::pause() {
    if (isPlayingState) {
        isPlayingState = false;
        return true;
    }
    return false;
}

bool PlayerProcessor::stop() {
    if (reader != nullptr) {
        isPlayingState = false;
        currentPosition = 0;
        resetAllEffects();
        return true;
    }
    return false;
}

uint64_t PlayerProcessor::seek(uint64_t sample) {
    if (reader != nullptr && sample < totalLength) {
        currentPosition = sample;
        resetAllEffects();
        return currentPosition;
    }
    return currentPosition;
}

void PlayerProcessor::addEffect(std::shared_ptr<EffectProcessor> effect) {
    if (effect) {
        effect->setSampleRate(currentSampleRate);
        effects.push_back(effect);
    }
}

void PlayerProcessor::removeEffect(std::shared_ptr<EffectProcessor> effect) {
    effects.erase(std::remove(effects.begin(), effects.end(), effect), effects.end());
}

void PlayerProcessor::clearEffects() {
    effects.clear();
}

void PlayerProcessor::resetAllEffects() {
    for (auto& effect : effects) {
        if (effect) {
            effect->resetState();
        }
    }
}

void PlayerProcessor::processingBlock(juce::AudioBuffer<float>& buffer) {
    // Apply all effects in the chain
    for (auto& effect : effects) {
        if (effect) {
            effect->processingBlock(buffer);
        }
    }
}

void PlayerProcessor::audioDeviceIOCallback(const float* const* /*inputChannelData*/, int /*numInputChannels*/,
                                           float* const* outputChannelData, int numOutputChannels, int numSamples) {
    // Clear output buffers
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        if (outputChannelData[ch] != nullptr) {
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
        }
    }

    // Resize process buffer if needed
    if (processBuffer.getNumSamples() != numSamples || processBuffer.getNumChannels() != static_cast<int>(reader->numChannels)) {
        processBuffer.setSize(reader->numChannels, numSamples);
        std::cout << "Resized process buffer to " << reader->numChannels
                  << " channels, " << numSamples << " samples" << std::endl;
    }

    int samplesToRead = numSamples;
    if (currentPosition + samplesToRead > totalLength) {
        samplesToRead = static_cast<int>(totalLength - currentPosition);
    }

    if (samplesToRead > 0) {
        // Read audio data directly into processBuffer
        bool readSuccess = reader->read(&processBuffer, 0, samplesToRead, static_cast<int>(currentPosition), true, true);
        
        if (!readSuccess) {
            std::cerr << "Failed to read audio data at position " << currentPosition << std::endl;
            return;
        }

        currentPosition += samplesToRead;

        // Apply effects
        processingBlock(processBuffer);
        
        for (int ch = 0; ch < numOutputChannels; ++ch) {
            int sourceChannel = std::min(ch, processBuffer.getNumChannels() - 1);
            
            if (outputChannelData[ch] != nullptr) {
                juce::FloatVectorOperations::copy(outputChannelData[ch],
                                                 processBuffer.getReadPointer(sourceChannel),
                                                 samplesToRead);
            }
        }

        // Stop playback if we reached the end
        if (currentPosition >= totalLength) {
            std::cout << "Reached end of audio file" << std::endl;
            isPlayingState = false;
            currentPosition = 0;
            resetAllEffects();
        }
    }
}

void PlayerProcessor::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    if (device != nullptr) {
        currentSampleRate = device->getCurrentSampleRate();
        currentBufferSize = device->getCurrentBufferSizeSamples();

        // Update all effects with new sample rate
        for (auto& effect : effects) {
            if (effect) {
                effect->setSampleRate(currentSampleRate);
            }
        }

        // Prepare processing buffer
        processBuffer.setSize(2, currentBufferSize); // Assume stereo output
    }
}

void PlayerProcessor::audioDeviceStopped() {
    // Clear processing buffer when device stops
    processBuffer.setSize(0, 0);
}
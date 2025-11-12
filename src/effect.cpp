#include "effect.h"
#include <cmath>

BiquadFilter::BiquadFilter() {
    calculateCoefficients();
}

void BiquadFilter::processingBlock(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Ensure state vectors have the right size
    if (x1.size() != static_cast<size_t>(numChannels)) {
        x1.resize(numChannels, 0.0);
        x2.resize(numChannels, 0.0);
        y1.resize(numChannels, 0.0);
        y2.resize(numChannels, 0.0);
    }
    
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* channelData = buffer.getWritePointer(ch);
        
        for (int i = 0; i < numSamples; ++i) {
            double input = static_cast<double>(channelData[i]);
            
            // Biquad difference equation
            double output = (b0 * input + b1 * x1[ch] + b2 * x2[ch] 
                           - a1 * y1[ch] - a2 * y2[ch]) / a0;
            
            // Update delay lines
            x2[ch] = x1[ch];
            x1[ch] = input;
            y2[ch] = y1[ch];
            y1[ch] = output;
            
            channelData[i] = static_cast<float>(output);
        }
    }
}

void BiquadFilter::resetState() {
    std::fill(x1.begin(), x1.end(), 0.0);
    std::fill(x2.begin(), x2.end(), 0.0);
    std::fill(y1.begin(), y1.end(), 0.0);
    std::fill(y2.begin(), y2.end(), 0.0);
}

void BiquadFilter::setSampleRate(double newSampleRate) {
    if (newSampleRate != sampleRate) {
        sampleRate = newSampleRate;
        calculateCoefficients();
        resetState();
    }
}

void BiquadFilter::setType(FilterType type) {
    if (type != filterType) {
        filterType = type;
        calculateCoefficients();
    }
}

void BiquadFilter::setFrequency(float newFrequency) {
    if (newFrequency != frequency) {
        frequency = newFrequency;
        calculateCoefficients();
    }
}

void BiquadFilter::setQ(float newQ) {
    if (newQ != q) {
        q = newQ;
        calculateCoefficients();
    }
}

void BiquadFilter::setGain(float newGain) {
    if (newGain != gain) {
        gain = newGain;
        calculateCoefficients();
    }
}

void BiquadFilter::calculateCoefficients() {
    const double omega = 2.0 * M_PI * frequency / sampleRate;
    const double sinw = std::sin(omega);
    const double cosw = std::cos(omega);
    const double alpha = sinw / (2.0 * q);

    switch (filterType) {
        case FilterType::BandPass:
            b0 = alpha;
            b1 = 0.0;
            b2 = -alpha;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw;
            a2 = 1.0 - alpha;

        case FilterType::Notch:
            b0 = 1.0;
            b1 = -2.0 * cosw;
            b2 = 1.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw;
            a2 = 1.0 - alpha;

        case FilterType::HighPass:
            b0 = (1.0 + cosw) / 2.0;
            b1 = -(1.0 + cosw);
            b2 = (1.0 + cosw) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw;
            a2 = 1.0 - alpha;

        case FilterType::LowPass:
            b0 = (1.0 - cosw) / 2.0;
            b1 = 1.0 - cosw;
            b2 = (1.0 - cosw) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosw;
            a2 = 1.0 - alpha;

    }

}
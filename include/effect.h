#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

class EffectProcessor {
public:
    virtual ~EffectProcessor() = default;
    
    // Process audio buffer in-place
    virtual void processingBlock(juce::AudioBuffer<float>& buffer) = 0;
    
    // Reset effect state (for filters with history, etc.)
    virtual void resetState() = 0;
    
    // Set sample rate for the effect
    virtual void setSampleRate(double sampleRate) = 0;
};

// Example Biquad filter implementation
class BiquadFilter : public EffectProcessor {
public:
    enum class FilterType {
        LowPass,
        HighPass,
        BandPass,
        Notch
    };
    
    BiquadFilter();
    ~BiquadFilter() override = default;
    
    void processingBlock(juce::AudioBuffer<float>& buffer) override;
    void resetState() override;
    void setSampleRate(double sampleRate) override;
    
    void setType(FilterType type);
    void setFrequency(float frequency);
    void setQ(float q);
    void setGain(float gain);
    
private:
    void calculateCoefficients();
    
    FilterType filterType = FilterType::LowPass;
    float frequency = 1000.0f;
    float q = 1.0f;
    float gain = 0.0f;
    double sampleRate = 44100.0;
    
    // Biquad coefficients
    double b0, b1, b2;
    double a0, a1, a2;
    
    // Filter state for each channel
    std::vector<double> x1, x2, y1, y2;
};

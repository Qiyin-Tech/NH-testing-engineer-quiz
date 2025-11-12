#include "player.h"
#include "effect.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <audio_file_path>" << std::endl;
        return -1;
    }
    
    // Create player instance
    PlayerProcessor player;
    
    // Initialize audio system
    if (!player.initializeAudio(44100.0, 1024)) {
        std::cerr << "Failed to initialize audio device!" << std::endl;
        return -1;
    }
    
    std::cout << "Audio player initialized successfully!" << std::endl;
    std::cout << "Sample rate: " << player.getSampleRate() << " Hz" << std::endl;

    auto highPassFilter = std::make_shared<BiquadFilter>();
    highPassFilter->setType(BiquadFilter::FilterType::HighPass);
    highPassFilter->setFrequency(120.0f); // 80Hz cutoff
    highPassFilter->setQ(0.707f);
    
    // Add effects to the player
    player.addEffect(highPassFilter);

    std::cout << "Added high-pass filters to the effects chain." << std::endl;
    
    // Load audio file from command line argument
    std::string audioFilePath = argv[1];
    if (!player.load(audioFilePath)) {
        std::cerr << "Failed to load audio file: \"" << audioFilePath << "\"" << std::endl;
        player.shutdownAudio();
        return -1;
    }
    
    std::cout << "Loaded audio file: " << audioFilePath << std::endl;
    std::cout << "Total samples: " << player.getTotalLength() << std::endl;
    
    // Start playback
    if (!player.play()) {
        std::cerr << "Failed to start playback!" << std::endl;
        player.shutdownAudio();
        return -1;
    }
    
    std::cout << "Playing audio... Press Enter to stop." << std::endl;
    
    // 启动一个后台线程来监控播放状态
    std::atomic<bool> shouldStop(false);
    std::thread monitorThread([&player, &shouldStop]() {
        while (!shouldStop.load()) {
            // 每2秒打印一次状态
            static int printCounter = 0;
            if (++printCounter >= 20) { // 2秒 = 20 * 100ms
                std::cout << "Playback status: " << (player.isPlaying() ? "Playing" : "Stopped")
                          << ", Position: " << player.getCurrentPosition()
                          << "/" << player.getTotalLength() << " samples" << std::endl;
                printCounter = 0;
            }
            
            // 检查是否播放完成
            if (!player.isPlaying() && player.getCurrentPosition() >= player.getTotalLength()) {
                std::cout << "Playback completed automatically." << std::endl;
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // Wait for user input to stop
    std::cin.get();
    
    // 停止监控线程
    shouldStop.store(true);
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    
    // Stop playback
    player.stop();
    std::cout << "Playback stopped." << std::endl;
    
    return 0;
}
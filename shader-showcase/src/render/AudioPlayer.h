#pragma once
#include <string>

class AudioPlayer {
public:
    bool Init(const std::string& audioPath);
    void Play();
    void Stop();
    void Destroy();
    bool IsPlaying() const;

private:
    void* m_handle = nullptr;  // MCI device handle
    bool m_initialized = false;
    bool m_playing = false;
};
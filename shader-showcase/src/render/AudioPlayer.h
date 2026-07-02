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
    void* m_device = nullptr;
    void* m_sound = nullptr;
    bool m_initialized = false;
};
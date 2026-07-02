#include "render/AudioPlayer.h"
#include <windows.h>
#include <mmsystem.h>
#include <cstdio>

#pragma comment(lib, "winmm.lib")

bool AudioPlayer::Init(const std::string& audioPath) {
    // Build MCI open command (use alias for subsequent control)
    std::string cmd = "open \"" + audioPath + "\" type mpegvideo alias VFXFireBook";
    MCIERROR err = mciSendStringA(cmd.c_str(), nullptr, 0, nullptr);
    if (err != 0) {
        // Try without type specification
        cmd = "open \"" + audioPath + "\" alias VFXFireBook";
        err = mciSendStringA(cmd.c_str(), nullptr, 0, nullptr);
        if (err != 0) {
            char buf[256];
            mciGetErrorStringA(err, buf, sizeof(buf));
            printf("[AudioPlayer] Failed to open audio: %s (error: %s)\n", audioPath.c_str(), buf);
            return false;
        }
    }
    m_initialized = true;
    printf("[AudioPlayer] Initialized: %s\n", audioPath.c_str());
    return true;
}

void AudioPlayer::Play() {
    if (!m_initialized) return;
    mciSendStringA("play VFXFireBook from 0", nullptr, 0, nullptr);
    m_playing = true;
    printf("[AudioPlayer] Playback started\n");
}

void AudioPlayer::Stop() {
    if (!m_initialized) return;
    mciSendStringA("stop VFXFireBook", nullptr, 0, nullptr);
    mciSendStringA("seek VFXFireBook to start", nullptr, 0, nullptr);
    m_playing = false;
    printf("[AudioPlayer] Playback stopped\n");
}

bool AudioPlayer::IsPlaying() const {
    return m_playing;
}

void AudioPlayer::Destroy() {
    if (!m_initialized) return;
    if (m_playing) {
        mciSendStringA("stop VFXFireBook", nullptr, 0, nullptr);
    }
    mciSendStringA("close VFXFireBook", nullptr, 0, nullptr);
    m_playing = false;
    m_initialized = false;
}
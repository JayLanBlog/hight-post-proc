#pragma once

// VideoPlayer — lightweight video frame reader using ffmpeg subprocess.
// Opens a video file, reads decoded RGBA frames via pipe from ffmpeg.
// No FFmpeg library linking required — just needs ffmpeg.exe on PATH.

#include <string>
#include <vector>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#define VIDEO_PIPE_TYPE HANDLE
#else
#define VIDEO_PIPE_TYPE int
#endif

class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();

    /// Open a video file for reading. Returns true on success.
    bool Open(const std::string& filePath);

    /// Read the next frame. Returns true if a NEW frame is available.
    /// Returns false if still accumulating (retry next tick).
    /// Use IsEnded() to check if the pipe has actually broken.
    bool ReadFrame();

    /// Returns true if the ffmpeg pipe has ended (video complete or error).
    bool IsEnded() const { return m_pipeEnded; }

    /// Seek to a specific time (seconds). Approximate.
    void Seek(double seconds);

    /// Close the video and release resources.
    void Close();

    /// Check if a video is currently open.
    bool IsOpen() const { return m_open; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    double GetDuration() const { return m_duration; }
    double GetCurrentTime() const { return m_currentTime; }
    const uint8_t* GetPixels() const { return m_pixels.data(); }
    double GetFPS() const { return m_fps; }

private:
    bool StartFFmpegProcess(const std::string& filePath);
    void StopProcess();

    VIDEO_PIPE_TYPE m_pipeRead  = nullptr;
#ifdef _WIN32
    HANDLE m_processHandle = nullptr;
#endif

    int    m_width       = 0;
    int    m_height      = 0;
    double m_duration    = 0.0;
    double m_fps         = 30.0;
    double m_currentTime = 0.0;
    bool   m_open        = false;
    bool   m_pipeEnded   = false;

    std::vector<uint8_t> m_pixels;
    std::vector<uint8_t> m_readBuf;  // raw pipe read buffer
    std::vector<uint8_t> m_accumBuf; // accumulation buffer for partial frames
    size_t m_accumLen = 0;

    static constexpr size_t READ_BUF_SIZE = 256 * 1024; // 256KB per chunk
};

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

    /// Read the next frame. Returns true if a new frame was read.
    /// After success, GetWidth()/GetHeight()/GetPixels() are valid.
    bool ReadFrame();

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

    std::vector<uint8_t> m_pixels;
    std::vector<uint8_t> m_readBuf;  // raw pipe read buffer

    static constexpr size_t READ_BUF_SIZE = 4 * 1024 * 1024; // 4MB read buffer
};

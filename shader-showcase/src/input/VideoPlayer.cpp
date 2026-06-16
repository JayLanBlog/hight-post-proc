#include "input/VideoPlayer.h"

#ifdef _WIN32
#include <windows.h>
#include <cstdio>
#include <cstring>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include <string>

// ============================================================================
// Constructor / Destructor
// ============================================================================

VideoPlayer::VideoPlayer() {
    m_readBuf.resize(READ_BUF_SIZE);
}

VideoPlayer::~VideoPlayer() {
    Close();
}

// ============================================================================
// Close
// ============================================================================

void VideoPlayer::Close() {
    StopProcess();
    m_open = false;
    m_pipeEnded = false;
    m_pixels.clear();
    m_accumLen = 0;
    m_width = 0;
    m_height = 0;
}

// ============================================================================
// Open
// ============================================================================

bool VideoPlayer::Open(const std::string& filePath) {
    Close();

    if (!StartFFmpegProcess(filePath)) {
        fprintf(stderr, "[VideoPlayer] Failed to start ffmpeg process\n");
        return false;
    }

    // Dimensions were already obtained by ffprobe in StartFFmpegProcess.
    // DO NOT call ReadFrame() here — ffmpeg needs time to decode the first
    // frame, and synchronous ReadFile() would block the UI thread.
    // The render loop will start reading frames on the first OnUpdate tick.

    m_open = true;
    printf("[VideoPlayer] Opened: %s (%d x %d, %.1f fps, %.1f sec)\n",
           filePath.c_str(), m_width, m_height, m_fps, m_duration);
    return true;
}

// ============================================================================
// ReadFrame — read RGBA frame data from ffmpeg pipe
// ============================================================================

bool VideoPlayer::ReadFrame() {
    if (!m_pipeRead || m_pipeEnded) return false;

    size_t needed = (size_t)m_width * m_height * 4;
    if (needed == 0) return false;

    // Ensure buffers are sized (Close() clears them, StartFFmpegProcess resizes m_pixels)
    if (m_pixels.size() < needed) m_pixels.resize(needed);
    if (m_accumBuf.size() < needed) m_accumBuf.resize(needed);

    // ---- Aggressive non-blocking drain loop ----
    // Peek + Read repeatedly until pipe is empty OR frame is complete.
    // This drains the pipe buffer as fast as possible each tick,
    // allowing ffmpeg to push more data between ticks.
    while (m_accumLen < needed) {
#ifdef _WIN32
        DWORD available = 0;
        if (!PeekNamedPipe(m_pipeRead, nullptr, 0, nullptr, &available, nullptr)) {
            m_pipeEnded = true;  // pipe broken — video ended
            return false;
        }
        if (available == 0) {
            return false;  // no data this tick, try again next frame
        }
        DWORD toRead = (DWORD)(needed - m_accumLen);
        if (toRead > available) toRead = available;
        if (toRead > (DWORD)m_readBuf.size()) toRead = (DWORD)m_readBuf.size();

        DWORD bytesRead = 0;
        if (!ReadFile(m_pipeRead, m_readBuf.data(), toRead, &bytesRead, nullptr)) {
            m_pipeEnded = true;
            return false;
        }
        if (bytesRead == 0) {
            m_pipeEnded = true;
            return false;
        }
#else
        ssize_t bytesRead = read((int)m_pipeRead, m_readBuf.data(),
            std::min(needed - m_accumLen, m_readBuf.size()));
        if (bytesRead <= 0) { m_pipeEnded = true; return false; }
#endif

        memcpy(m_accumBuf.data() + m_accumLen, m_readBuf.data(), bytesRead);
        m_accumLen += bytesRead;
    }

    // ---- Full frame accumulated — copy to output ----
    memcpy(m_pixels.data(), m_accumBuf.data(), needed);
    m_accumLen = 0;

    m_currentTime += 1.0 / m_fps;
    return true;
}

// ============================================================================
// Seek — restart ffmpeg with seek offset
// ============================================================================

void VideoPlayer::Seek(double seconds) {
    // We can't seek in a pipe-based reader.
    // This is a no-op for the simple implementation.
    (void)seconds;
}

// ============================================================================
// StartFFmpegProcess (Windows)
// ============================================================================

#ifdef _WIN32

bool VideoPlayer::StartFFmpegProcess(const std::string& filePath) {
    // First, probe video info using ffprobe
    char probeCmd[1024];
    snprintf(probeCmd, sizeof(probeCmd),
        "ffprobe -v error -select_streams v:0 -show_entries "
        "stream=width,height,r_frame_rate,duration -of csv=p=0 \"%s\"",
        filePath.c_str());

    FILE* probePipe = _popen(probeCmd, "r");
    if (probePipe) {
        char line[512];
        if (fgets(line, sizeof(line), probePipe)) {
            // Parse: width,height,fps_num/fps_den,duration
            int w = 0, h = 0;
            float fpsNum = 0, fpsDen = 1;
            double dur = 0;
            if (sscanf(line, "%d,%d,%f/%f,%lf", &w, &h, &fpsNum, &fpsDen, &dur) >= 2) {
                m_width = w;
                m_height = h;
                if (fpsDen > 0) m_fps = fpsNum / fpsDen;
                m_duration = dur;
            }
        }
        _pclose(probePipe);
    }

    // If probe failed, try with defaults
    if (m_width == 0 || m_height == 0) {
        m_width = 1920;
        m_height = 1080;
        m_fps = 30.0;
        m_duration = 10.0;
    }

    // Resize pixel buffer
    m_pixels.resize((size_t)m_width * m_height * 4);

    // Create pipe for reading ffmpeg output
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        fprintf(stderr, "[VideoPlayer] CreatePipe failed\n");
        return false;
    }

    // Set read handle to non-inheritable
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    // Build ffmpeg command
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "ffmpeg -i \"%s\" -loglevel error -f rawvideo -pix_fmt rgba "
        "-s %dx%d -r 30 pipe:1",
        filePath.c_str(), m_width, m_height);

    // Create process
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.hStdOutput = hWrite;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {};

    if (!CreateProcessA(nullptr, cmd, nullptr, nullptr, TRUE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        fprintf(stderr, "[VideoPlayer] CreateProcess failed for ffmpeg\n");
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return false;
    }

    CloseHandle(hWrite);  // Close write end in parent
    CloseHandle(pi.hThread);

    m_pipeRead = hRead;
    m_processHandle = pi.hProcess;
    return true;
}

void VideoPlayer::StopProcess() {
    if (m_pipeRead) {
        CloseHandle((HANDLE)m_pipeRead);
        m_pipeRead = nullptr;
    }
    if (m_processHandle) {
        TerminateProcess(m_processHandle, 0);
        WaitForSingleObject(m_processHandle, 1000);
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
}

#else

bool VideoPlayer::StartFFmpegProcess(const std::string& filePath) {
    // Unix implementation using popen
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "ffprobe -v error -select_streams v:0 -show_entries "
        "stream=width,height,r_frame_rate,duration -of csv=p=0 '%s'",
        filePath.c_str());

    FILE* probePipe = popen(cmd, "r");
    if (probePipe) {
        char line[512];
        if (fgets(line, sizeof(line), probePipe)) {
            int w = 0, h = 0;
            float fpsNum = 0, fpsDen = 1;
            double dur = 0;
            if (sscanf(line, "%d,%d,%f/%f,%lf", &w, &h, &fpsNum, &fpsDen, &dur) >= 2) {
                m_width = w;
                m_height = h;
                if (fpsDen > 0) m_fps = fpsNum / fpsDen;
                m_duration = dur;
            }
        }
        pclose(probePipe);
    }

    if (m_width == 0 || m_height == 0) {
        m_width = 1920;
        m_height = 1080;
        m_fps = 30.0;
    }

    snprintf(cmd, sizeof(cmd),
        "ffmpeg -i '%s' -loglevel error -f rawvideo -pix_fmt rgba pipe:1",
        filePath.c_str());

    FILE* pipe = popen(cmd, "r");
    if (!pipe) return false;

    m_pipeRead = (VIDEO_PIPE_TYPE)fileno(pipe);
    return true;
}

void VideoPlayer::StopProcess() {
    if (m_pipeRead) {
        close((int)m_pipeRead);
        m_pipeRead = nullptr;
    }
}

#endif

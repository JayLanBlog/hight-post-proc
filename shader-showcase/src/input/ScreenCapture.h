#pragma once

// DXGI Desktop Duplication screen capture
// Captures the primary monitor and provides frames as RGBA8 pixel data
// for uploading to OpenGL textures.

#ifdef _WIN32
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#endif

#include <vector>
#include <cstdint>
#include <string>

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();

    /// Initialize DXGI Desktop Duplication for the primary monitor.
    /// Returns true on success.
    bool Init();

    /// Shutdown and release all DXGI/D3D11 resources.
    void Shutdown();

    /// Capture one frame from the desktop.
    /// Returns true if a new frame was acquired.
    /// After a successful call, GetWidth()/GetHeight()/GetPixels() are valid.
    bool CaptureFrame();

    /// Get captured frame width.
    int GetWidth() const { return m_width; }
    /// Get captured frame height.
    int GetHeight() const { return m_height; }
    /// Get pointer to RGBA8 pixel data (valid after successful CaptureFrame).
    const uint8_t* GetPixels() const { return m_pixels.data(); }

    /// True if initialized and ready to capture.
    bool IsReady() const { return m_ready; }

private:
#ifdef _WIN32
    ID3D11Device*           m_d3dDevice        = nullptr;
    ID3D11DeviceContext*    m_d3dContext        = nullptr;
    IDXGIOutputDuplication* m_deskDupl          = nullptr;
    ID3D11Texture2D*        m_stagingTex        = nullptr;
#endif

    int m_width  = 0;
    int m_height = 0;
    bool m_ready = false;

    std::vector<uint8_t> m_pixels;
};

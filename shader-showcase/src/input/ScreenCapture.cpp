#include "input/ScreenCapture.h"

#ifdef _WIN32
#include <cstdio>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// ============================================================================
// Helper: find the adapter that owns the primary monitor output
// ============================================================================
static IDXGIOutput* FindPrimaryOutput(IDXGIFactory1* factory, IDXGIAdapter1*& outAdapter) {
    outAdapter = nullptr;
    for (UINT i = 0; ; i++) {
        IDXGIAdapter1* adapter = nullptr;
        if (factory->EnumAdapters1(i, &adapter) == DXGI_ERROR_NOT_FOUND)
            break;

        for (UINT j = 0; ; j++) {
            IDXGIOutput* output = nullptr;
            if (adapter->EnumOutputs(j, &output) == DXGI_ERROR_NOT_FOUND)
                break;

            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);

            if (desc.AttachedToDesktop) {
                outAdapter = adapter; // caller owns adapter ref
                printf("[ScreenCapture] Found output: %ls (%d x %d)\n",
                       desc.DeviceName, desc.DesktopCoordinates.right - desc.DesktopCoordinates.left,
                       desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top);
                return output;
            }
            output->Release();
        }
        adapter->Release();
    }
    return nullptr;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

ScreenCapture::ScreenCapture() = default;

ScreenCapture::~ScreenCapture() {
    Shutdown();
}

// ============================================================================
// Init
// ============================================================================

bool ScreenCapture::Init() {
    if (m_ready) return true;

    // ---- Create D3D11 device ----
    D3D_FEATURE_LEVEL featLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,                          // default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,                                // no flags
        nullptr, 0,                       // no feature levels specified
        D3D11_SDK_VERSION,
        &m_d3dDevice,
        &featLevel,
        &m_d3dContext);

    if (FAILED(hr)) {
        fprintf(stderr, "[ScreenCapture] D3D11CreateDevice failed: 0x%08X\n", (unsigned)hr);
        return false;
    }
    printf("[ScreenCapture] D3D11 device created (feature level 0x%x)\n", (unsigned)featLevel);

    // ---- Get DXGI Factory ----
    IDXGIDevice* dxgiDevice = nullptr;
    hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) { fprintf(stderr, "[ScreenCapture] IDXGIDevice QI failed\n"); Shutdown(); return false; }

    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiDevice->Release();
    if (FAILED(hr)) { fprintf(stderr, "[ScreenCapture] GetAdapter failed\n"); Shutdown(); return false; }

    IDXGIFactory1* factory = nullptr;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)&factory);
    dxgiAdapter->Release();
    if (FAILED(hr)) { fprintf(stderr, "[ScreenCapture] GetParent failed\n"); Shutdown(); return false; }

    // Find primary output via adapter enumeration
    IDXGIAdapter1* foundAdapter = nullptr;
    IDXGIOutput* output = FindPrimaryOutput(factory, foundAdapter);
    factory->Release();

    if (!output) {
        fprintf(stderr, "[ScreenCapture] No desktop output found\n");
        if (foundAdapter) foundAdapter->Release();
        Shutdown();
        return false;
    }

    // Get Output1 for DuplicateOutput
    IDXGIOutput1* output1 = nullptr;
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
    output->Release();
    if (FAILED(hr)) { fprintf(stderr, "[ScreenCapture] IDXGIOutput1 QI failed\n"); Shutdown(); return false; }

    // ---- Duplicate output ----
    hr = output1->DuplicateOutput(m_d3dDevice, &m_deskDupl);
    output1->Release();
    if (FAILED(hr)) {
        fprintf(stderr, "[ScreenCapture] DuplicateOutput failed: 0x%08X\n", (unsigned)hr);
        Shutdown();
        return false;
    }

    // Get output dimensions from the duplication interface
    DXGI_OUTDUPL_DESC duplDesc;
    m_deskDupl->GetDesc(&duplDesc);
    m_width  = duplDesc.ModeDesc.Width;
    m_height = duplDesc.ModeDesc.Height;

    printf("[ScreenCapture] Desktop duplication started (%d x %d)\n", m_width, m_height);

    // Allocate pixel buffer
    m_pixels.resize(m_width * m_height * 4);

    m_ready = true;
    return true;
}

// ============================================================================
// Shutdown
// ============================================================================

void ScreenCapture::Shutdown() {
    if (m_stagingTex) { m_stagingTex->Release(); m_stagingTex = nullptr; }
    if (m_deskDupl)   { m_deskDupl->Release();   m_deskDupl   = nullptr; }
    if (m_d3dContext) { m_d3dContext->Release(); m_d3dContext = nullptr; }
    if (m_d3dDevice)  { m_d3dDevice->Release();  m_d3dDevice  = nullptr; }
    m_ready = false;
    printf("[ScreenCapture] Shutdown\n");
}

// ============================================================================
// CaptureFrame
// ============================================================================

bool ScreenCapture::CaptureFrame() {
    if (!m_ready) return false;

    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    HRESULT hr = m_deskDupl->AcquireNextFrame(16, &frameInfo, &desktopResource);

    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        // No new frame available — return previous frame data (still valid)
        // But we need to mark that there's no new data
        return false;
    }

    if (FAILED(hr)) {
        // DXGI_ERROR_ACCESS_LOST means we need to re-create duplication
        if (hr == DXGI_ERROR_ACCESS_LOST) {
            fprintf(stderr, "[ScreenCapture] Access lost, re-initializing...\n");
            Shutdown();
            return Init();
        }
        fprintf(stderr, "[ScreenCapture] AcquireNextFrame failed: 0x%08X\n", (unsigned)hr);
        return false;
    }

    // Get the desktop texture
    ID3D11Texture2D* desktopTex = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopTex);
    desktopResource->Release();
    if (FAILED(hr)) {
        m_deskDupl->ReleaseFrame();
        fprintf(stderr, "[ScreenCapture] QI for ID3D11Texture2D failed\n");
        return false;
    }

    // Create staging texture on first capture
    if (!m_stagingTex) {
        D3D11_TEXTURE2D_DESC desc = {};
        desktopTex->GetDesc(&desc);
        desc.Usage          = D3D11_USAGE_STAGING;
        desc.BindFlags      = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags      = 0;
        hr = m_d3dDevice->CreateTexture2D(&desc, nullptr, &m_stagingTex);
        if (FAILED(hr)) {
            fprintf(stderr, "[ScreenCapture] CreateTexture2D (staging) failed: 0x%08X\n", (unsigned)hr);
            desktopTex->Release();
            m_deskDupl->ReleaseFrame();
            return false;
        }
    }

    // Copy desktop texture to staging
    m_d3dContext->CopyResource(m_stagingTex, desktopTex);
    desktopTex->Release();

    // Map staging texture and read pixels
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_d3dContext->Map(m_stagingTex, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        m_deskDupl->ReleaseFrame();
        fprintf(stderr, "[ScreenCapture] Map failed: 0x%08X\n", (unsigned)hr);
        return false;
    }

    // Copy with BGRA → RGBA conversion and row-pitch handling
    const uint8_t* src = static_cast<const uint8_t*>(mapped.pData);
    uint8_t* dst = m_pixels.data();
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            // BGRA → RGBA
            dst[(y * m_width + x) * 4 + 0] = src[y * mapped.RowPitch + x * 4 + 2]; // R ← B
            dst[(y * m_width + x) * 4 + 1] = src[y * mapped.RowPitch + x * 4 + 1]; // G ← G
            dst[(y * m_width + x) * 4 + 2] = src[y * mapped.RowPitch + x * 4 + 0]; // B ← R
            dst[(y * m_width + x) * 4 + 3] = src[y * mapped.RowPitch + x * 4 + 3]; // A ← A
        }
    }

    m_d3dContext->Unmap(m_stagingTex, 0);
    m_deskDupl->ReleaseFrame();

    return true;
}

#else
// Non-Windows stub
ScreenCapture::ScreenCapture() = default;
ScreenCapture::~ScreenCapture() = default;
bool ScreenCapture::Init() { return false; }
void ScreenCapture::Shutdown() {}
bool ScreenCapture::CaptureFrame() { return false; }
#endif

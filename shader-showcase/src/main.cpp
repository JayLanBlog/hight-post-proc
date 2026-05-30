// Auto-test mode: cycles through all 18 cards.
// Set AUTO_TEST=1 to enable, or remove for normal mode.

#include "app/Application.h"
#include "app/CoverFlowScene.h"
#include "render/IRenderBackend.h"
#include "render/OpenGLBackend.h"
#include "stb_image.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstdint>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "stb_image.h"

static std::string FindAssetDir() {
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        std::string exePath(buf, (size_t)len);
        auto sl = exePath.find_last_of("\\/");
        if (sl != std::string::npos) exePath = exePath.substr(0, sl);
        for (const char* rel : {"../../../assets", "../../assets", "../assets", "assets"}) {
            std::string test = exePath + "/" + rel + "/test.jpg";
            FILE* f = fopen(test.c_str(), "rb");
            if (f) { fclose(f); return exePath + "/" + rel; }
        }
    }
#endif
    return "assets";
}

int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    // Check for auto-test mode via environment variable
    const char* autoTestEnv = getenv("AUTO_TEST");
    bool autoTest = (autoTestEnv && strcmp(autoTestEnv, "1") == 0);
    if (getenv("AUTO_TEST_DETAILS") && strcmp(getenv("AUTO_TEST_DETAILS"), "1") == 0) {
        autoTest = true;
    }

    Application app;

    app.SetFrameCallback([&](float dt) {
        (void)dt;
        // Re-create scene if it was destroyed (e.g., after backend switch)
        if (app.GetCurrentScene() != nullptr) return;
        // Only recreate scene for OpenGL backend (Vulkan scene not supported yet)
        if (app.GetBackendType() != BackendType::OpenGL) return;

        IRenderBackend* backend = app.GetBackend();
        if (!backend) return;

        std::string assetDir = FindAssetDir();
        std::string jpgPath  = assetDir + "/test.jpg";
        int iw = 0, ih = 0, comp = 0;
        stbi_set_flip_vertically_on_load(true); // Flip for OpenGL bottom-left origin
        stbi_uc* imgData = stbi_load(jpgPath.c_str(), &iw, &ih, &comp, 4);
        stbi_set_flip_vertically_on_load(false);
        if (!imgData) {
            fprintf(stderr, "[main] Cannot load test image: %s\n", jpgPath.c_str());
            return;
        }
        printf("[main] Loaded image: %s (%d x %d)\n", jpgPath.c_str(), iw, ih);

        TextureHandle inputTex = backend->CreateTexture(iw, ih, TextureFormat::RGBA8, imgData);
        stbi_image_free(imgData);

        const char* testImageList[] = {
            "00_grayscale_landscape.jpg",   // simple_test (Grayscale)
            "01_bloom_citynight.jpg",       // bloom
            "02_blur_brickwall.jpg",        // blur
            "03_sharpen_architecture.jpg",  // sharpen
            "04_edge_building.jpg",         // edge_detect
            "05_emboss_metal.jpg",          // emboss
            "06_pixelate_portrait.jpg",     // pixelate
            "07_vignette_flower.jpg",       // vignette
            "08_chromatic_leaves.jpg",      // chromatic
            "09_colorgrading_food.jpg",     // color_grade
            "10_noise_sky.jpg",             // noise
            "11_kaleidoscope_mandala.jpg",  // kaleidoscope
            "12_glitch_tech.jpg",           // glitch
            "13_toon_cartoon.jpg",          // toon
            "14_vhs_retro.jpg",             // vhs
            "15_crt_screen.jpg",            // crt
            "16_water_lake.jpg",            // water_ripple
            "17_lens_wideangle.jpg"         // lens_distort
        };
        const int NUM_EFFECTS = (int)(sizeof(testImageList) / sizeof(testImageList[0]));

        std::vector<TextureHandle> inputTexCache;  // cache per-effect input textures
        inputTexCache.reserve(NUM_EFFECTS);

        // Find test images directory (relative to executable)
        std::string testImageBaseDir;
#ifdef _WIN32
        char buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            std::string exePath(buf, (size_t)len);
            auto sl = exePath.find_last_of("\\/");
            if (sl != std::string::npos) exePath = exePath.substr(0, sl);
            // Try to find assets/images relative to exe (new location)
            for (const char* rel : {"../../../assets/images", "../../assets/images",
                                    "../assets/images", "assets/images",
                                    "../../../screenshots/assets/images", "../../screenshots/assets/images",
                                    "../screenshots/assets/images", "screenshots/assets/images"}) {
                std::string test = exePath + "/" + rel + "/00_grayscale_landscape.jpg";
                FILE* f = fopen(test.c_str(), "rb");
                if (f) { fclose(f); testImageBaseDir = exePath + "/" + rel + "/"; break; }
            }
        }
#endif
        if (testImageBaseDir.empty()) {
            testImageBaseDir = "assets/images/";
        }
        printf("[main] Test images directory: %s\n", testImageBaseDir.c_str());

        // Load and cache input textures for each effect
        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string testImgPath = testImageBaseDir + testImageList[i];
            int tiw = 0, tih = 0, tcomp = 0;
            stbi_set_flip_vertically_on_load(true);
            stbi_uc* testImgData = stbi_load(testImgPath.c_str(), &tiw, &tih, &tcomp, 4);
            stbi_set_flip_vertically_on_load(false);

            TextureHandle effectInputTex = inputTex; // fallback to default
            if (testImgData) {
                printf("[main] Loaded test image for effect %d: %s (%d x %d)\n", i, testImgPath.c_str(), tiw, tih);
                effectInputTex = backend->CreateTexture(tiw, tih, TextureFormat::RGBA8, testImgData);
                stbi_image_free(testImgData);
            } else {
                printf("[main] Warning: Cannot load test image %s, using default\n", testImgPath.c_str());
            }
            inputTexCache.push_back(effectInputTex);
        }

        printf("[main] All %d input textures cached\n", (int)inputTexCache.size());

        auto coverFlow = std::make_unique<CoverFlowScene>();
        coverFlow->SetBackend(backend);
        coverFlow->SetInputTexture(inputTex);
        coverFlow->SetInputTexCache(inputTexCache);
        coverFlow->SetTestImageBaseDir(testImageBaseDir);
        coverFlow->SetApplication(&app);
        coverFlow->AddImageToPool(jpgPath);
        coverFlow->AddImageToPool(assetDir + "/portrait.jpg");
        coverFlow->AddImageToPool(assetDir + "/nature.jpg");
        coverFlow->AddImageToPool(assetDir + "/abstract.jpg");

        // Add test images to pool for Ctrl+Left/Right cycling
        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string imgPath = testImageBaseDir + testImageList[i];
            FILE* f = fopen(imgPath.c_str(), "rb");
            if (f) {
                fclose(f);
                coverFlow->AddImageToPool(imgPath);
            }
        }

        // Add test videos to pool
        const char* testVideoList[] = {
            "00_grayscale.mp4", "01_bloom.mp4", "02_blur.mp4", "03_sharpen.mp4",
            "04_edge.mp4", "05_emboss.mp4", "06_pixelate.mp4", "07_vignette.mp4",
            "08_chromatic.mp4", "09_colorgrade.mp4", "10_noise.mp4", "11_kaleidoscope.mp4",
            "12_glitch.mp4", "13_toon.mp4", "14_vhs.mp4", "15_crt.mp4",
            "16_water.mp4", "17_lens.mp4"
        };
        std::string testVideoBaseDir;
#ifdef _WIN32
        if (len > 0 && len < MAX_PATH) {
            std::string exePath(buf, (size_t)len);
            auto sl = exePath.find_last_of("\\/");
            if (sl != std::string::npos) exePath = exePath.substr(0, sl);
            for (const char* rel : {"../../../assets/videos", "../../assets/videos",
                                    "../assets/videos", "assets/videos",
                                    "../../../screenshots/assets/videos", "../../screenshots/assets/videos",
                                    "../screenshots/assets/videos", "screenshots/assets/videos"}) {
                std::string test = exePath + "/" + rel + "/00_grayscale.mp4";
                FILE* f = fopen(test.c_str(), "rb");
                if (f) { fclose(f); testVideoBaseDir = exePath + "/" + rel + "/"; break; }
            }
        }
#endif
        if (testVideoBaseDir.empty()) {
            testVideoBaseDir = "assets/videos/";
        }
        printf("[main] Test videos directory: %s\n", testVideoBaseDir.c_str());

        for (int i = 0; i < NUM_EFFECTS; i++) {
            std::string vidPath = testVideoBaseDir + testVideoList[i];
            FILE* f = fopen(vidPath.c_str(), "rb");
            if (f) {
                fclose(f);
                coverFlow->AddVideoToPool(vidPath);
            }
        }

        if (autoTest) {
            coverFlow->EnableAutoTest(80); // auto-cycle every card, hold 80 frames each
        }

        app.SetScene(std::move(coverFlow));
        printf("[main] CoverFlowScene started (autoTest=%d)\n", autoTest);
    });

    return app.Run(argc, argv);
}

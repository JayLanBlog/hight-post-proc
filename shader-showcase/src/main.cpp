// Auto-test mode: cycles through all 18 cards.
// Set AUTO_TEST=1 to enable, or remove for normal mode.

#include "app/Application.h"
#include "app/CoverFlowScene.h"
#include "app/AUS3DScene.h"
#include "app/LiquidGlassScene.h"
#include "app/VFXFireBookScene.h"
#include "app/SceneRegistry.h"
#include "app/SceneGalleryScene.h"
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
#include <direct.h>   // _chdir, _fullpath
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

    // Set working directory to project root so relative paths (assets/images/*.jpg, etc.) resolve
#ifdef _WIN32
    {
        char buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            std::string exePath(buf, (size_t)len);
            auto sl = exePath.find_last_of("\\/");
            if (sl != std::string::npos) {
                std::string exeDir = exePath.substr(0, sl);
                for (const char* up : {"../../..", "../..", "..", "."}) {
                    std::string test = exeDir + "/" + up + "/assets/images/00_grayscale_landscape.jpg";
                    FILE* f = fopen(test.c_str(), "rb");
                    if (f) {
                        fclose(f);
                        std::string root = exeDir + "/" + up;
                        char absRoot[MAX_PATH];
                        if (_fullpath(absRoot, root.c_str(), MAX_PATH)) {
                            _chdir(absRoot);
                            printf("[main] Working directory set to: %s\n", absRoot);
                        }
                        break;
                    }
                }
            }
        }
    }
#endif

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

        // ================================================================
        // Register "后处理特效" (CoverFlowScene) with a reusable factory.
        // ALL captured variables MUST be by-VALUE — the frame callback's
        // local variables are destroyed after first invocation. References
        // to them are dangling when the factory runs later.
        // ================================================================
        auto* be     = backend;
        auto  tex    = inputTex;
        // Copy by value — these are local variables that go out of scope
        auto  cache  = inputTexCache;            // copy vector
        auto  imgDir = testImageBaseDir;          // copy string
        auto  ad     = assetDir;                  // copy string
        auto  jpg    = jpgPath;                   // copy string
        auto  ne     = NUM_EFFECTS;

        // Find test video directory
        std::string testVideoBaseDir;
#ifdef _WIN32
        {
            char buf2[MAX_PATH];
            DWORD len2 = GetModuleFileNameA(nullptr, buf2, MAX_PATH);
            if (len2 > 0 && len2 < MAX_PATH) {
                std::string ep(buf2, (size_t)len2);
                auto sl = ep.find_last_of("\\/");
                if (sl != std::string::npos) ep = ep.substr(0, sl);
                for (const char* rel : {"../../../assets/videos","../../assets/videos","../assets/videos","assets/videos"}) {
                    std::string test = ep + "/" + rel + "/00_grayscale.mp4";
                    FILE* f = fopen(test.c_str(), "rb");
                    if (f) { fclose(f); testVideoBaseDir = ep + "/" + rel + "/"; break; }
                }
            }
        }
#endif
        if (testVideoBaseDir.empty()) testVideoBaseDir = "assets/videos/";
        printf("[main] Test videos directory: %s\n", testVideoBaseDir.c_str());
        auto  vdDir  = testVideoBaseDir;          // copy string

        // Capture EVERYTHING by value (including copies of vectors/strings)
        SceneRegistry::Instance().Register({
            "post-processing",
            "后处理特效",
            "后处理",
            "91 种 GPU 实时后处理效果：模糊、辉光、故障、CRT…",
            "assets/images/00_grayscale_landscape.jpg",
            [be, tex, cache, imgDir, ad, jpg, ne, vdDir, autoTest, &app]() -> std::unique_ptr<Scene> {
                printf("[main] Factory: creating new CoverFlowScene\n");
                auto c = std::make_unique<CoverFlowScene>();
                c->SetBackend(be);
                c->SetInputTexture(tex);
                c->SetInputTexCache(cache);
                c->SetTestImageBaseDir(imgDir);
                c->SetApplication(&app);
                c->AddImageToPool(jpg);
                c->AddImageToPool(ad + "/portrait.jpg");
                c->AddImageToPool(ad + "/nature.jpg");
                c->AddImageToPool(ad + "/abstract.jpg");
                {
                    const char* testImageList[] = {
                        "00_grayscale_landscape.jpg","01_bloom_citynight.jpg","02_blur_brickwall.jpg",
                        "03_sharpen_architecture.jpg","04_edge_building.jpg","05_emboss_metal.jpg",
                        "06_pixelate_portrait.jpg","07_vignette_flower.jpg","08_chromatic_leaves.jpg",
                        "09_colorgrading_food.jpg","10_noise_sky.jpg","11_kaleidoscope_mandala.jpg",
                        "12_glitch_tech.jpg","13_toon_cartoon.jpg","14_vhs_retro.jpg",
                        "15_crt_screen.jpg","16_water_lake.jpg","17_lens_wideangle.jpg"
                    };
                    for (int i = 0; i < ne; i++) {
                        std::string p = imgDir + testImageList[i];
                        FILE* f = fopen(p.c_str(), "rb");
                        if (f) { fclose(f); c->AddImageToPool(p); }
                    }
                }
                {
                    const char* testVideoList[] = {
                        "00_grayscale.mp4","01_bloom.mp4","02_blur.mp4","03_sharpen.mp4",
                        "04_edge.mp4","05_emboss.mp4","06_pixelate.mp4","07_vignette.mp4",
                        "08_chromatic.mp4","09_colorgrade.mp4","10_noise.mp4","11_kaleidoscope.mp4",
                        "12_glitch.mp4","13_toon.mp4","14_vhs.mp4","15_crt.mp4",
                        "16_water.mp4","17_lens.mp4"
                    };
                    for (int i = 0; i < ne; i++) {
                        std::string v = vdDir + testVideoList[i];
                        FILE* f = fopen(v.c_str(), "rb");
                        if (f) { fclose(f); c->AddVideoToPool(v); }
                    }
                }
                if (autoTest) c->EnableAutoTest(80);
                printf("[main] Factory: CoverFlowScene created\n");
                return c;
            },
            true
        });

        // Register AUS 3D Object Shader page (separate from post-processing)
        SceneRegistry::Instance().Register({
            "aus-3d",
            "AUS 3D物体",
            "AUS 3D",
            "58 种 3D 物体着色器：卡通、高光、边缘光、车漆…",
            "assets/images/00_grayscale_landscape.jpg",
            [be, &app]() -> std::unique_ptr<Scene> {
                auto s = std::make_unique<AUS3DScene>();
                s->SetBackend(be);
                s->SetApplication(&app);
                return s;
            },
            true
        });

        // Register LiquidGlass page
        SceneRegistry::Instance().Register({
            "liquid-glass",
            "Liquid Glass",
            "特效",
            "Apple Liquid Glass 风格玻璃效果 — 超椭圆SDF折射+高斯模糊+发光",
            "assets/images/00_grayscale_landscape.jpg",
            [be, &app]() -> std::unique_ptr<Scene> {
                auto s = std::make_unique<LiquidGlassScene>();
                s->SetBackend(be);
                s->SetApplication(&app);
                return s;
            },
            true
        });

        // Register VFX Fire Book page
        SceneRegistry::Instance().Register({
            "vfx-fire-book",
            "VFX Fire Book",
            "VFX",
            "Magic Fire Book - 3D书本+溶解+粒子+音频",
            "assets/images/00_grayscale_landscape.jpg",
            [be, &app]() -> std::unique_ptr<Scene> {
                auto s = std::make_unique<VFXFireBookScene>();
                s->SetBackend(be);
                s->SetApplication(&app);
                return s;
            },
            true
        });

        auto gallery = std::make_unique<SceneGalleryScene>();
        gallery->SetApplication(&app);

        // AUTO_TEST_CARDS: skip gallery, launch CoverFlowScene directly for screenshots
        if (getenv("AUTO_TEST_CARDS") && !getenv("AUTO_TEST_UI")) {
            printf("[main] AUTO_TEST_CARDS mode: launching CoverFlowScene directly\n");
            auto c = std::make_unique<CoverFlowScene>();
            c->SetBackend(backend);
            c->SetInputTexture(inputTex);
            c->SetInputTexCache(inputTexCache);
            c->SetTestImageBaseDir(testImageBaseDir);
            c->SetApplication(&app);
            c->AddImageToPool(assetDir + "/test.jpg");
            if (autoTest) c->EnableAutoTest(80);
            app.SetScene(std::move(c));
        } else if (getenv("AUTO_TEST_AUS3D")) {
            printf("[main] AUTO_TEST_AUS3D mode: launching AUS3DScene directly\n");
            auto s = std::make_unique<AUS3DScene>();
            s->SetBackend(backend);
            s->SetApplication(&app);
            app.SetScene(std::move(s));
        } else if (getenv("LIQUID_GLASS")) {
            printf("[main] LIQUID_GLASS mode: launching LiquidGlassScene directly\n");
            auto s = std::make_unique<LiquidGlassScene>();
            s->SetBackend(backend);
            s->SetApplication(&app);
            app.SetScene(std::move(s));
        } else {
            app.SetScene(std::move(gallery));
        }
        printf("[main] SceneGalleryScene started (autoTest=%d)\n", autoTest);
    });

    return app.Run(argc, argv);
}

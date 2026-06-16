@echo off
echo === Shader Showcase Auto Build and Screenshot ===
echo.

echo [1/4] Killing old ShaderShowcase processes...
taskkill /F /IM ShaderShowcase.exe 2>nul
timeout /t 3 /nobreak >nul

echo [2/4] Building...
cd /d "e:\AI\graph\hight-post-proc\shader-showcase\build"
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo BUILD FAILED!
    pause
    exit /b 1
)

echo [3/4] Running and capturing 18 detail screenshots...
set AUTO_TEST_DETAILS=1
bin\Release\ShaderShowcase.exe

echo [4/4] Converting screenshots to PNG...
cd /d "e:\AI\graph\hight-post-proc"
python -c "from PIL import Image; import os; [Image.open(f'screenshots/detail_{i:02d}.ppm').save(f'screenshots/detail_{i:02d}.png') for i in range(18) if os.path.exists(f'screenshots/detail_{i:02d}.ppm')]"

echo.
echo === Done! Screenshots saved to e:\AI\graph\hight-post-proc\screenshots\ ===
echo.
pause

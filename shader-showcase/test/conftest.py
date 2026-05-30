# e:\AI\graph\hight-post-proc\shader-showcase\test\conftest.py
"""Pytest fixtures for Shader Showcase automated testing."""
import pytest
import subprocess
import time
import os
import sys
import pyautogui

# Resolve paths relative to this file
TEST_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(TEST_DIR)
ASSET_DIR = os.path.join(PROJECT_DIR, "assets", "test")
SCREENSHOT_DIR = os.path.join(PROJECT_DIR, "test_screenshots")
EXE_DIR = os.path.join(PROJECT_DIR, "build", "bin", "Release")
EXE_PATH = os.path.join(EXE_DIR, "ShaderShowcase.exe")


@pytest.fixture(scope="session", autouse=True)
def ensure_resources():
    """Ensure test resources exist before any test runs."""
    os.makedirs(ASSET_DIR, exist_ok=True)
    os.makedirs(SCREENSHOT_DIR, exist_ok=True)
    try:
        subprocess.run(
            [sys.executable, "-c",
             "from test.test_resources import download_all; download_all()"],
            cwd=PROJECT_DIR,
            timeout=120,
            capture_output=True
        )
    except Exception as e:
        print(f"[WARN] Resource download: {e}")


class AppProcess:
    """Simple wrapper for the ShaderShowcase process."""

    def __init__(self, proc):
        self.proc = proc
        self._start_time = time.time()

    def is_running(self):
        return self.proc.poll() is None

    def is_visible(self):
        return self.is_running()

    def exists(self):
        return self.is_running()

    def uptime(self):
        return time.time() - self._start_time


@pytest.fixture(scope="function")
def app_window():
    """Start the ShaderShowcase app for each test.

    Key: do NOT pipe stdout/stderr — GLFW windows behave differently
    when output is captured. Let the process write to its own console.
    """
    # Kill any existing instances
    try:
        subprocess.run(
            ["taskkill", "/F", "/IM", "ShaderShowcase.exe"],
            capture_output=True, timeout=5
        )
        time.sleep(1)
    except Exception:
        pass

    if not os.path.exists(EXE_PATH):
        pytest.fail(f"Executable not found: {EXE_PATH}")

    env = os.environ.copy()
    env["PATH"] = EXE_DIR + os.pathsep + env.get("PATH", "")

    # Start process WITHOUT capturing stdout/stderr.
    # Do NOT use CREATE_NEW_CONSOLE — it can cause the GLFW window
    # to be created on a different desktop that pyautogui can't reach.
    proc = subprocess.Popen(
        [EXE_PATH],
        cwd=EXE_DIR,
        env=env,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    app = AppProcess(proc)

    # Wait for shader compilation + thumbnail rendering (~10s)
    print(f"[FIXTURE] Process started (PID={proc.pid}), waiting for init...")
    for i in range(20):
        time.sleep(1)
        if not app.is_running():
            pytest.fail(f"Process exited during init (after {i+1}s)")
        if i >= 10:
            break

    pyautogui.PAUSE = 0.2

    # Try to activate the GLFW window so pyautogui clicks land on it
    try:
        import pygetwindow as gw
        time.sleep(1)
        target = None
        for w in gw.getAllWindows():
            try:
                if "Shader Showcase" in (w.title or ""):
                    target = w
                    break
            except:
                continue
        if target:
            target.activate()
            time.sleep(0.5)
            try:
                target.moveTo(100, 50)
                target.resize(1400, 900)
            except:
                pass
            print(f"[FIXTURE] Activated window: '{target.title}' at ({target.left},{target.top})")
        else:
            print("[FIXTURE] WARNING: 'Shader Showcase' window not found")
            # List all windows for debugging
            for w in gw.getAllWindows():
                try:
                    if w.title:
                        print(f"[FIXTURE]   Found window: '{w.title}'")
                except:
                    pass
    except Exception as e:
        print(f"[FIXTURE] Window activation warning: {e}")

    print(f"[FIXTURE] Process ready (uptime={app.uptime():.1f}s)")

    yield app

    # Cleanup
    try:
        proc.terminate()
        proc.wait(timeout=5)
    except Exception:
        try:
            proc.kill()
        except Exception:
            pass

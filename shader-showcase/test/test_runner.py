# e:\AI\graph\hight-post-proc\shader-showcase\test\test_runner.py
"""Run all automated tests and generate HTML report."""
import subprocess, sys, os

PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def main():
    # Ensure resources first
    print("=== Step 1: Ensuring test resources ===")
    result = subprocess.run(
        [sys.executable, "-c",
         "from test.test_resources import download_all; download_all()"],
        cwd=PROJECT_DIR,
        capture_output=True, text=True, timeout=120
    )
    print(result.stdout)
    if result.stderr:
        print("[STDERR]", result.stderr)
    if result.returncode != 0:
        print(f"[WARN] Resource download exited {result.returncode}")

    # Run pytest
    print("\n=== Step 2: Running pytest ===")
    os.makedirs(os.path.join(PROJECT_DIR, "test_screenshots"), exist_ok=True)
    report_path = os.path.join(PROJECT_DIR, "test_screenshots", "report.html")

    pytest_args = [
        sys.executable, "-m", "pytest",
        os.path.join(PROJECT_DIR, "test", "test_automated.py"),
        "-v", "--tb=short",
        f"--html={report_path}",
        "--self-contained-html",
        "--capture=no",  # show print output
    ]

    result = subprocess.run(pytest_args, cwd=PROJECT_DIR)
    print(f"\nPytest exit code: {result.returncode}")
    print(f"Report: {report_path}")
    return result.returncode

if __name__ == "__main__":
    sys.exit(main())

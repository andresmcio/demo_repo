# 🧪 Lab: SCANOSS Demo
This repository is designed to showcase the full spectrum of SCANOSS capabilities in a multi-language environment.

## 🛠️ How to Demo SCANOSS Tools

### 1. CLI & Snippet Detection
Run: `scanoss-py scan .`
* **What to show:** Contrast the MIT root license with the **GPL-2.0** found in `vendor/networking.c`.
* **Value:** Shows that we scan "DNA", not just declared metadata.

### 2. Crypto Finder
Run: `scanoss-py scan . --identify crypto`
* **Look at:** `src/core_logic.c`. 
* **Value:** Highlights internal implementations of AES without relying on standard libraries.

### 3. Pre-commit Hooks
1. Edit any file and add a "bad" snippet (e.g., a line from OpenSSL).
2. Try to `git commit`. 
* **Value:** Shows "Shift-Left" security. The scan happens *before* code leaves the dev machine.

### 4. GitHub Actions
* Create a Pull Request with a new file in `vendor/`.
* **Value:** Show the automated comment in the PR blocking the merge due to license policy violation.

### 5. Dependency Tracker
* Check the `SBOM` output for `scancode-toolkit` results.
* **Value:** Show how we track transitive dependencies that dev teams often ignore.

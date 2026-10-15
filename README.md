# InstallZone

<div align="center">

![InstallZone Logo](assets/icons/installzone-studio.svg)

### Professional Modular Scriptable Installer System for Linux
*Targeting Kubuntu 26.04.1 LTS with KDE Plasma & Modern Linux Distributions*

[English](docs/README_en.md) | [Українська](docs/README_uk.md) | [日本語](docs/README_ja.md) | [简体中文](docs/README_zh.md)

[Scripting Guide (EN)](docs/SCRIPTING_en.md) | [Довідник скриптів (UK)](docs/SCRIPTING_uk.md) | [スクリプトガイド (JA)](docs/SCRIPTING_ja.md) | [脚本指南 (ZH)](docs/SCRIPTING_zh.md)

</div>

---

## 📖 Overview

**InstallZone** is a C++17 modular application packaging and installation system tailored for Linux, with special attention to Kubuntu 26.04.1 LTS and the KDE Plasma desktop environment.

It uses **OpenGL 2.1** and **Dear ImGui** for maximum graphics hardware compatibility, paired with an embedded **JavaScript** scripting engine (Duktape) for executing advanced setup routines—such as generating user configuration directories (`~/.config/`), configuring dependencies like `navi`, and creating XDG FreeDesktop shortcuts.

```
┌─────────────────────────────────────────────────────────────┐
│                    InstallZone Ecosystem                    │
├───────────────────────────────┬─────────────────────────────┤
│   App 1: InstallZone Studio   │   App 2: Base Runner        │
│   (Installer Creator & IDE)   │   (Self-Extracting Wizard)  │
├───────────────────────────────┴─────────────────────────────┤
│            Shared Core (libinstallzone_core)                │
│   ┌──────────────┐ ┌──────────────┐ ┌───────────────────┐   │
│   │ Project JSON │ │  JS Engine   │ │ Archive Container │   │
│   │ Serialization│ │  (Duktape)   │ │ Pack & Extract    │   │
│   └──────────────┘ └──────────────┘ └───────────────────┘   │
│   ┌──────────────┐ ┌──────────────┐ ┌───────────────────┐   │
│   │ Multilingual │ │  JPL Safety  │ │ System & Desktop  │   │
│   │ i18n (12 Ls) │ │  Framework   │ │ Polkit Elevation  │   │
│   └──────────────┘ └──────────────┘ └───────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│         Backend: SDL2 + OpenGL 2.1 + Dear ImGui             │
└─────────────────────────────────────────────────────────────┘
```

---

## ✨ Features

- **Application 1: InstallZone Studio (`installzone-studio`)**:
  - Full graphical interface to define projects, files, and dependencies.
  - JSON project format (`.izproj`) for version-controllable installer projects.
  - Interactive JavaScript editors with pre-configured templates (e.g., `~/.config` setup, `navi` integration).
  - Standalone installer builder: stitches the base runner with the compressed payload into an executable `.run` file with CRC32 integrity verification.
- **Application 2: InstallZone Base Runner (`installzone-runner`)**:
  - Bundled as the runtime for all packed installers.
  - Reads self-extracting payload directly from the executable.
  - Automatic Polkit/sudo elevation prompts when target directories require root permissions.
  - Real-time progress bar, file count, and scrollable log viewer.
  - Generates uninstaller records and standalone `uninstall.sh`.
- **JPL (NASA/Jet Propulsion Laboratory) Software Standard Compliance**:
  - Bounded iteration loops (`kMaxLoopIterations`) preventing infinite hangs.
  - Bounded queue memory models for log buffers and data transfers.
  - Explicit function return status checking (`iz::StatusCode`).
  - No recursion; iterative path traversal and extraction.
- **Zero ImGui Context Loss Threading Architecture**:
  - UI rendering is strictly isolated to the main thread.
  - Heavy file I/O, extraction, and JavaScript logic run on background worker threads.
  - Thread-safe `WorkerBridge` guarantees smooth 60 FPS UI without lockups or context corruption.
- **12 Supported Languages**:
  - English (`en`), Ukrainian (`uk`), Japanese (`ja`), Chinese (`zh`), Spanish (`es`), French (`fr`), German (`de`), Portuguese (`pt`), Italian (`it`), Polish (`pl`), Korean (`ko`), Russian (`ru`).
- **Clean Root Directory**:
  - Only `install.sh`, `reinstall.sh`, `uninstall.sh`, `README.md`, and Git configuration in the root directory.

---

## 🚀 Installation & Usage

### 1. Build and Install

```bash
# Builds both applications and installs to ~/.local/bin or /usr/local/bin
./install.sh
```

### 2. Reinstall or Uninstall

```bash
# Clean reinstall
./reinstall.sh

# Complete removal
./uninstall.sh
```

### 3. Launching

Launch from the KDE Plasma Application Menu (**InstallZone Studio**) or execute:
```bash
installzone-studio
```

---

## 📄 License

InstallZone is licensed under the MIT License.

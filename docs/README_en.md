# InstallZone - Professional Modular Scriptable Installer System

> **A high-reliability, JPL-inspired Linux installer suite built with C++17, OpenGL2, Dear ImGui, and embedded JavaScript.**  
> Focused on Kubuntu 26.04.1 LTS with KDE Plasma.

---

## 🌟 Key Highlights

- **Dual-Application Architecture**:
  1. **InstallZone Studio (`installzone-studio`)**: Creator tool for assembling files, managing dependencies, configuring FreeDesktop shortcuts, and authoring pre/post JavaScript hooks. Packs everything into a standalone executable `.run` installer.
  2. **InstallZone Base Runner (`installzone-runner`)**: Lightweight standalone installer wizard that reads the embedded payload, checks dependencies, requests Polkit/sudo elevation when required, extracts files with full permission preservation, executes JavaScript configurations, and creates uninstaller records.
- **Embedded JavaScript Engine**: Full ECMAScript scripting layer (via Duktape) for setting up `~/.config` folders, configuring tools like `navi`, writing environment files, and running system tasks.
- **Strict JPL Coding Standards**: Safety-critical principles inspired by the NASA/JPL Institutional Coding Standard (bounded loops, bounded memory queues, assertions, explicit return status handling, no recursion).
- **Asynchronous Multithreaded UI**: The UI thread and logic worker thread are completely decoupled. The UI operates at 60 FPS without ever locking during heavy I/O, while avoiding ImGui context loss by strictly confining all ImGui rendering to the main thread.
- **Out-of-the-Box Localization**: Full native translation for 12 major languages:
  English, Ukrainian, Japanese, Chinese, Spanish, French, German, Portuguese, Italian, Polish, Korean, and Russian.
- **Clean Project Structure**: Only `install.sh`, `reinstall.sh`, `uninstall.sh`, and Git files reside in the root directory.

---

## 🚀 Quick Start

### Installation

```bash
# Build and install to ~/.local (or /usr/local if run as root)
./install.sh
```

### Reinstallation & Uninstallation

```bash
# Rebuild and reinstall cleanly
./reinstall.sh

# Remove installed binaries and shortcuts
./uninstall.sh
```

### Launching

- From the application launcher: search for **InstallZone Studio**
- From terminal:
  ```bash
  installzone-studio
  ```

---

## 🛠️ Packaging an Application

1. Open **InstallZone Studio**.
2. **General**: Enter your App Name, Version, ID, and default installation directory (`/opt/myapp` or `~/.local/opt/myapp`).
3. **Files**: Add your binaries, configuration templates, and assets.
4. **Dependencies**: Add system or apt packages (e.g., `navi`, `curl`, `libsdl2`).
5. **Scripts**: Customize JavaScript hooks (e.g. creating `~/.config/navi/cheats`).
6. **Desktop**: Specify launcher shortcut names and icons.
7. **Pack**: Click **"Pack Standalone Installer"** to create a self-contained `.run` binary.

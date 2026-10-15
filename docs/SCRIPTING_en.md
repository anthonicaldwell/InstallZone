# InstallZone Scripting Reference (English)

InstallZone features an embedded JavaScript scripting engine (powered by Duktape) to automate advanced installation, system configuration, environment setup, and dependency initialization (such as creating `~/.config` folders or setting up tool configurations like `navi`).

---

## 1. Execution Lifecycle & Hooks

InstallZone provides four JavaScript execution hooks:

1. **Pre-Install Hook (`pre_install_js`)**:
   - Executes **before** files are extracted to the target directory.
   - Use cases: verifying custom pre-conditions, stopping background daemons or services, backing up existing configuration files.

2. **Post-Install Hook (`post_install_js`)**:
   - Executes **after** files are unpacked and desktop launcher shortcuts are installed.
   - Use cases: creating user configuration directories (e.g., `~/.config/<app>`), installing tool integrations (e.g. `navi` cheatsheets, shell auto-completions), setting system environment variables, creating symlinks.

3. **Pre-Uninstall Hook (`pre_uninstall_js`)**:
   - Executes **before** files are deleted during uninstallation.
   - Use cases: stopping running services, unregistering system services.

4. **Post-Uninstall Hook (`post_uninstall_js`)**:
   - Executes **after** files, desktop entries, and icons have been removed.
   - Use cases: cleaning up cache files or asking whether to remove user configs.

---

## 2. Global Object: `iz`

The runtime injects the global object `iz` into the JavaScript execution context.

### Properties

| Property | Type | Description |
| :--- | :--- | :--- |
| `iz.app_name` | `string` | The display name of the application. |
| `iz.version` | `string` | The version string (e.g., `"1.0.0"`). |
| `iz.install_dir` | `string` | The absolute path where files are being installed (e.g., `"/opt/myapp"`). |
| `iz.home_dir` | `string` | The real user's home directory (e.g., `"/home/username"`). Correctly detects real user even when running with `sudo` or `pkexec`. |
| `iz.config_dir` | `string` | The user's configuration folder (e.g., `"/home/username/.config"`). |
| `iz.is_uninstall` | `boolean` | `true` if currently running an uninstall routine; `false` during install. |

---

### Methods

#### `iz.log(message)`
Outputs a log message to the installer console and installer log window in real time.
```javascript
iz.log("Starting post-installation configuration...");
```

#### `iz.mkdir(path)`
Recursively creates directories (`mkdir -p`). Tilde (`~/`) is automatically expanded to the real user's home directory. When running elevated under root, file ownership is automatically transferred to the real desktop user.
```javascript
var cfgDir = iz.config_dir + "/myapp";
iz.mkdir(cfgDir);
```

#### `iz.write_file(path, content)`
Writes text content to the specified file. Automatically creates any missing parent directories. Sets proper user ownership if running elevated.
```javascript
var configFile = iz.config_dir + "/myapp/settings.json";
iz.write_file(configFile, JSON.stringify({ theme: "dark", telemetry: false }, null, 2));
```

#### `iz.read_file(path)`
Reads and returns the contents of a text file as a string. Returns `null` if the file cannot be opened.
```javascript
var content = iz.read_file(iz.config_dir + "/myapp/settings.json");
if (content) {
    iz.log("Read configuration: " + content);
}
```

#### `iz.file_exists(path)`
Returns `true` if the file or directory exists; otherwise `false`.
```javascript
if (!iz.file_exists(iz.home_dir + "/.bashrc")) {
    iz.log("Bashrc not found.");
}
```

#### `iz.remove(path)`
Deletes the file or empty directory at `path`. Returns boolean success.
```javascript
iz.remove(iz.config_dir + "/myapp/old_cache.tmp");
```

#### `iz.copy(src, dst)`
Copies a file from `src` to `dst`.
```javascript
iz.copy(iz.install_dir + "/share/default.conf", iz.config_dir + "/myapp/app.conf");
```

#### `iz.exec(command)`
Executes a system shell command and returns an object containing the exit code and standard output.
```javascript
var result = iz.exec("which navi");
if (result.exit_code === 0) {
    iz.log("Found navi binary at: " + result.stdout.trim());
}
```

#### `iz.get_env(name)` & `iz.set_env(name, value)`
Reads or sets an environment variable.

---

## 3. Practical Example: Navi Configuration Setup

When an application depends on `navi` or provides command-line cheatsheets, you can configure it automatically in `post_install_js`:

```javascript
iz.log("Setting up Navi cheatsheets for " + iz.app_name + "...");

var naviCheatsDir = iz.home_dir + "/.config/navi/cheats";
iz.mkdir(naviCheatsDir);

var cheatContent = 
    "% " + iz.app_name + "\n" +
    "# Launch " + iz.app_name + " with verbose output\n" +
    iz.app_name.toLowerCase() + " --verbose\n\n" +
    "# Reset application state\n" +
    iz.app_name.toLowerCase() + " --reset-config\n";

iz.write_file(naviCheatsDir + "/" + iz.app_name.toLowerCase() + ".cheat", cheatContent);
iz.log("Navi configuration finished successfully!");
```

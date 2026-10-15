# InstallZone 脚本编写指南 (简体中文)

InstallZone 内置了基于 Duktape 的轻量级高可靠 JavaScript 脚本引擎。借助脚本层，您可以在安装/卸载过程中执行高级配置操作，例如自动创建 `~/.config` 用户目录、为 `navi` 等工具生成配置与速查表、设置环境变量等。

---

## 1. 执行生命周期与钩子 (Hooks)

InstallZone 提供四个独立的 JavaScript 执行钩子：

1. **安装前钩子 (`pre_install_js`)**:
   - 在文件解压到目标目录**之前**执行。
   - 典型用途：检查特定的系统环境、停止可能冲突的后台进程或服务、备份现有配置。

2. **安装后钩子 (`post_install_js`)**:
   - 在文件解压完成并且桌面快捷方式安装**之后**执行。
   - 典型用途：创建用户配置目录（例如 `~/.config/<应用名称>`）、为 `navi` 生成快捷备忘录、设置系统环境配置、创建符号链接。

3. **卸载前钩子 (`pre_uninstall_js`)**:
   - 在删除文件**之前**执行。
   - 典型用途：停止正在运行的守护进程或服务。

4. **卸载后钩子 (`post_uninstall_js`)**:
   - 在所有文件和桌面图标移除**之后**执行。
   - 典型用途：清理临时缓存或根据需要清理配置文件。

---

## 2. 全局对象：`iz`

系统自动向 JavaScript 上下文中注入名为 `iz` 的全局对象。

### 属性列表

| 属性 | 类型 | 说明 |
| :--- | :--- | :--- |
| `iz.app_name` | `string` | 应用程序的显示名称。 |
| `iz.version` | `string` | 版本号字符串（如 `"1.0.0"`）。 |
| `iz.install_dir` | `string` | 应用程序安装目录的绝对路径（如 `"/opt/myapp"`）。 |
| `iz.home_dir` | `string` | 实际登录用户的家目录（如 `"/home/username"`）。即使使用 `sudo` 或 `pkexec` 提权运行，也能准确检测出真实用户路径。 |
| `iz.config_dir` | `string` | 用户的配置目录路径（`~/.config`）。 |
| `iz.is_uninstall` | `boolean` | 当前是否正在执行卸载流程（卸载时为 `true`，安装时为 `false`）。 |

---

### 方法列表

#### `iz.log(message)`
在安装程序界面日志窗口以及控制台实时输出日志信息。
```javascript
iz.log("开始执行安装后配置...");
```

#### `iz.mkdir(path)`
递归创建目录（类似于 `mkdir -p`）。路径中的波浪号（`~/`）会自动解析为用户的真实家目录。在 root 提权运行时，创建的目录所有权会自动设置为普通用户。
```javascript
var myConfig = iz.config_dir + "/myapp";
iz.mkdir(myConfig);
```

#### `iz.write_file(path, content)`
将文本写入指定文件。若父目录不存在会自动创建。如果提权运行，会自动修正文件权限。
```javascript
var settingsFile = iz.config_dir + "/myapp/config.json";
iz.write_file(settingsFile, JSON.stringify({ theme: "dark" }, null, 2));
```

#### `iz.read_file(path)`
读取并返回文本文件内容。若文件无法打开则返回 `null`。

#### `iz.file_exists(path)`
检测指定路径的文件或目录是否存在，返回布尔值。

#### `iz.remove(path)`
删除指定路径的文件或空目录。

#### `iz.copy(src, dst)`
将文件从 `src` 复制到 `dst`。

#### `iz.exec(command)`
执行 Linux Shell 命令，并返回包含退出码（`exit_code`）和标准输出（`stdout`）的对象。
```javascript
var check = iz.exec("which navi");
if (check.exit_code === 0) {
    iz.log("已找到 navi 命令: " + check.stdout.trim());
}
```

---

## 3. 实战示例：Navi 依赖项配置

当您的应用程序依赖 `navi` 或者希望提供交互式速查表时，可以在 `post_install_js` 中自动配置：

```javascript
iz.log("正在为 " + iz.app_name + " 配置 Navi cheatsheet...");

var naviDir = iz.home_dir + "/.config/navi/cheats";
iz.mkdir(naviDir);

var cheat = 
    "% " + iz.app_name + "\n" +
    "# 启动 " + iz.app_name + "\n" +
    iz.app_name.toLowerCase() + " --verbose\n\n" +
    "# 重置配置\n" +
    iz.app_name.toLowerCase() + " --reset\n";

iz.write_file(naviDir + "/" + iz.app_name.toLowerCase() + ".cheat", cheat);
iz.log("Navi 配置已顺利完成！");
```

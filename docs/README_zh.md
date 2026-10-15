# InstallZone - 专业模块化可脚本化安装程序系统 (简体中文)

> **基于 C++17、OpenGL2、Dear ImGui 与内置 JavaScript 构建的高可靠 Linux 安装程序套件，遵循 JPL 严苛编码标准。**  
> 重点针对 Kubuntu 26.04.1 LTS (KDE Plasma) 进行优化。

---

## 🌟 核心特性

- **双应用架构**:
  1. **InstallZone Studio (`installzone-studio`)**: 安装包制作工作台，支持打包所需文件、配置系统/Apt依赖项、定制 FreeDesktop 桌面图标，以及编写安装前后 JavaScript 脚本。一键打包输出单一自解压 `.run` 二进制可执行文件。
  2. **InstallZone Base Runner (`installzone-runner`)**: 基础安装向导程序，负责解析内嵌安装包、检查依赖项、在需要时通过 Polkit/sudo 请求管理员提权、保留文件权限安全解压、执行 JS 配置脚本并自动生成卸载器。
- **内置 JavaScript 引擎**: 借助 Duktape 引擎，轻松处理 `~/.config` 目录创建、`navi` 交互式速查表配置、环境变量与复杂环境准备。
- **遵循 JPL (NASA/喷气推进实验室) 软件标准**: 所有循环均有严格上限，杜绝死循环；严格限制无节制的动态内存增长；明确的错误码校验；函数模块化且具备丰富断言保护。
- **UI 与业务逻辑完全多线程解耦**: 彻底避免 ImGui 多线程上下文丢失（Context Loss）。UI 线程独占渲染通道并保持 60 FPS 流畅交互；文件 I/O 与解压均在工作线程中进行。
- **开箱即用的 12 种主要语言本地化支持**:
  简体中文、英语、乌克兰语、日语、西班牙语、法语、德语、葡萄牙语、意大利语、波兰语、韩语、俄语。

---

## 🚀 快速入门

### 安装软件

```bash
# 构建并安装到 ~/.local（若为 root 则安装到 /usr/local）
./install.sh
```

### 重新安装与卸载

```bash
# 重新编译并安装
./reinstall.sh

# 从系统中完全移除
./uninstall.sh
```

### 启动程序

- 从 KDE Plasma 应用程序启动器中搜索：**InstallZone Studio**
- 或在终端中执行：
  ```bash
  installzone-studio
  ```

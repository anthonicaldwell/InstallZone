#include "iz_i18n.h"
#include "imgui.h"
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

namespace iz {

Language I18n::s_current_lang = Language::English;

static const std::vector<LanguageInfo> s_languages = {
    { Language::English,    "en", "English",    "English" },
    { Language::Ukrainian,  "uk", "Ukrainian",  "Українська" },
    { Language::Japanese,   "ja", "Japanese",   "日本語" },
    { Language::Chinese,    "zh", "Chinese",    "简体中文" },
    { Language::Spanish,    "es", "Spanish",    "Español" },
    { Language::French,     "fr", "French",     "Français" },
    { Language::German,     "de", "German",     "Deutsch" },
    { Language::Portuguese, "pt", "Portuguese", "Português" },
    { Language::Italian,    "it", "Italian",    "Italiano" },
    { Language::Polish,     "pl", "Polish",     "Polski" },
    { Language::Korean,     "ko", "Korean",     "한국어" },
    { Language::Russian,    "ru", "Russian",    "Русский" }
};

// String tables: [lang_index][key] -> translation
static std::unordered_map<std::string, std::string> s_tables[static_cast<int>(Language::Count)];

static void AddTranslation(Language lang, const char* key, const char* val) {
    s_tables[static_cast<int>(lang)][key] = val;
}

void I18n::Initialize() {
    // English (Default)
    AddTranslation(Language::English, "app_title_studio", "InstallZone Studio - Installer Creator");
    AddTranslation(Language::English, "app_title_runner", "InstallZone - Application Installer");
    AddTranslation(Language::English, "tab_general", "General Info");
    AddTranslation(Language::English, "tab_files", "Files & Assets");
    AddTranslation(Language::English, "tab_dependencies", "Dependencies");
    AddTranslation(Language::English, "tab_scripts", "JavaScript Scripts");
    AddTranslation(Language::English, "tab_desktop", "Desktop Integration");
    AddTranslation(Language::English, "tab_pack", "Pack Installer");
    AddTranslation(Language::English, "btn_new", "New Project");
    AddTranslation(Language::English, "btn_open", "Open Project");
    AddTranslation(Language::English, "btn_save", "Save Project");
    AddTranslation(Language::English, "btn_save_as", "Save As...");
    AddTranslation(Language::English, "btn_exit", "Exit");
    AddTranslation(Language::English, "btn_pack_installer", "Pack Standalone Installer");
    AddTranslation(Language::English, "btn_add_file", "Add File");
    AddTranslation(Language::English, "btn_add_directory", "Add Directory");
    AddTranslation(Language::English, "btn_remove", "Remove Selected");
    AddTranslation(Language::English, "btn_clear", "Clear All");
    AddTranslation(Language::English, "btn_add_dep", "Add Dependency");
    AddTranslation(Language::English, "btn_test_dep", "Check Dependency");
    AddTranslation(Language::English, "btn_test_script", "Test Run Script (Sandbox)");
    AddTranslation(Language::English, "btn_next", "Next >");
    AddTranslation(Language::English, "btn_back", "< Back");
    AddTranslation(Language::English, "btn_install", "Install Now");
    AddTranslation(Language::English, "btn_uninstall", "Uninstall");
    AddTranslation(Language::English, "btn_reinstall", "Reinstall");
    AddTranslation(Language::English, "btn_cancel", "Cancel");
    AddTranslation(Language::English, "btn_finish", "Finish");
    AddTranslation(Language::English, "btn_browse", "Browse...");
    AddTranslation(Language::English, "btn_elevate", "Elevate with Sudo / Polkit");
    AddTranslation(Language::English, "lbl_app_name", "Application Name:");
    AddTranslation(Language::English, "lbl_app_id", "Application ID (e.g. org.app.name):");
    AddTranslation(Language::English, "lbl_version", "Version:");
    AddTranslation(Language::English, "lbl_author", "Author / Vendor:");
    AddTranslation(Language::English, "lbl_description", "Description:");
    AddTranslation(Language::English, "lbl_category", "Category:");
    AddTranslation(Language::English, "lbl_license", "License:");
    AddTranslation(Language::English, "lbl_install_prefix", "Default Install Directory:");
    AddTranslation(Language::English, "lbl_status", "Status:");
    AddTranslation(Language::English, "lbl_progress", "Progress:");
    AddTranslation(Language::English, "lbl_source_path", "Source Path");
    AddTranslation(Language::English, "lbl_target_path", "Destination Subpath");
    AddTranslation(Language::English, "lbl_permissions", "Permissions");
    AddTranslation(Language::English, "lbl_dep_name", "Dependency Name:");
    AddTranslation(Language::English, "lbl_dep_package", "Apt Package Name:");
    AddTranslation(Language::English, "lbl_dep_check_cmd", "Check Command:");
    AddTranslation(Language::English, "lbl_dep_required", "Required for Installation");
    AddTranslation(Language::English, "lbl_pre_install_js", "Pre-Install JavaScript Hook:");
    AddTranslation(Language::English, "lbl_post_install_js", "Post-Install JavaScript Hook (e.g. ~/.config setup, navi):");
    AddTranslation(Language::English, "lbl_pre_uninstall_js", "Pre-Uninstall JavaScript Hook:");
    AddTranslation(Language::English, "lbl_post_uninstall_js", "Post-Uninstall JavaScript Hook:");
    AddTranslation(Language::English, "lbl_desktop_name", "Desktop Entry Name:");
    AddTranslation(Language::English, "lbl_desktop_exec", "Exec Command / Binary:");
    AddTranslation(Language::English, "lbl_desktop_icon", "Icon Name / Path:");
    AddTranslation(Language::English, "lbl_desktop_categories", "Categories (e.g. Utility;Development;):");
    AddTranslation(Language::English, "lbl_desktop_terminal", "Run in Terminal");
    AddTranslation(Language::English, "lbl_select_output", "Output Installer Binary (.run):");
    AddTranslation(Language::English, "lbl_packing", "Packing installer payload...");
    AddTranslation(Language::English, "lbl_ready", "Ready.");
    AddTranslation(Language::English, "lbl_installed", "Installed successfully!");
    AddTranslation(Language::English, "lbl_installing", "Installing components...");
    AddTranslation(Language::English, "lbl_uninstalling", "Removing components...");
    AddTranslation(Language::English, "lbl_welcome", "Welcome to the installation wizard.");
    AddTranslation(Language::English, "lbl_destination", "Choose Destination Folder:");
    AddTranslation(Language::English, "lbl_space_required", "Required Space:");
    AddTranslation(Language::English, "lbl_elevation_needed", "Administrator privileges are required for this path.");
    AddTranslation(Language::English, "lbl_elevation_prompt", "Please authenticate via Polkit to continue installation.");
    AddTranslation(Language::English, "lbl_deps_checking", "Verifying system dependencies...");
    AddTranslation(Language::English, "lbl_deps_ok", "All required dependencies are satisfied.");
    AddTranslation(Language::English, "lbl_deps_missing", "Some dependencies are missing:");
    AddTranslation(Language::English, "lbl_install_complete", "Installation has finished successfully.");
    AddTranslation(Language::English, "lbl_uninstall_complete", "Uninstallation has finished successfully.");
    AddTranslation(Language::English, "lbl_launch_app", "Launch Application");
    AddTranslation(Language::English, "lbl_language", "Language");
    AddTranslation(Language::English, "lbl_help", "Help");
    AddTranslation(Language::English, "lbl_about", "About InstallZone");
    AddTranslation(Language::English, "lbl_about_desc", "InstallZone - Professional JPL-inspired Modular Scriptable Installer System.\nBackend: OpenGL2 + Dear ImGui.\nScripting: Embedded JavaScript engine.\nTarget: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::English, "lbl_sample_navi", "Insert Navi Config Snippet");
    AddTranslation(Language::English, "lbl_sample_mkdir", "Insert ~/.config Snippet");

    // Ukrainian (Українська)
    AddTranslation(Language::Ukrainian, "app_title_studio", "InstallZone Studio - Створення інсталяторів");
    AddTranslation(Language::Ukrainian, "app_title_runner", "InstallZone - Встановлення додатку");
    AddTranslation(Language::Ukrainian, "tab_general", "Загальна інформація");
    AddTranslation(Language::Ukrainian, "tab_files", "Файли та ресурси");
    AddTranslation(Language::Ukrainian, "tab_dependencies", "Залежності");
    AddTranslation(Language::Ukrainian, "tab_scripts", "JavaScript скрипти");
    AddTranslation(Language::Ukrainian, "tab_desktop", "Інтеграція робочого столу");
    AddTranslation(Language::Ukrainian, "tab_pack", "Створити інсталятор");
    AddTranslation(Language::Ukrainian, "btn_new", "Новий проект");
    AddTranslation(Language::Ukrainian, "btn_open", "Відкрити проект");
    AddTranslation(Language::Ukrainian, "btn_save", "Зберегти проект");
    AddTranslation(Language::Ukrainian, "btn_save_as", "Зберегти як...");
    AddTranslation(Language::Ukrainian, "btn_exit", "Вихід");
    AddTranslation(Language::Ukrainian, "btn_pack_installer", "Створити автономний інсталятор");
    AddTranslation(Language::Ukrainian, "btn_add_file", "Додати файл");
    AddTranslation(Language::Ukrainian, "btn_add_directory", "Додати теку");
    AddTranslation(Language::Ukrainian, "btn_remove", "Видалити обране");
    AddTranslation(Language::Ukrainian, "btn_clear", "Очистити все");
    AddTranslation(Language::Ukrainian, "btn_add_dep", "Додати залежність");
    AddTranslation(Language::Ukrainian, "btn_test_dep", "Перевірити залежність");
    AddTranslation(Language::Ukrainian, "btn_test_script", "Тестовий запуск (Пісочниця)");
    AddTranslation(Language::Ukrainian, "btn_next", "Далі >");
    AddTranslation(Language::Ukrainian, "btn_back", "< Назад");
    AddTranslation(Language::Ukrainian, "btn_install", "Встановити");
    AddTranslation(Language::Ukrainian, "btn_uninstall", "Видалити");
    AddTranslation(Language::Ukrainian, "btn_reinstall", "Перевстановити");
    AddTranslation(Language::Ukrainian, "btn_cancel", "Скасувати");
    AddTranslation(Language::Ukrainian, "btn_finish", "Завершити");
    AddTranslation(Language::Ukrainian, "btn_browse", "Огляд...");
    AddTranslation(Language::Ukrainian, "btn_elevate", "Підвищити права через Sudo / Polkit");
    AddTranslation(Language::Ukrainian, "lbl_app_name", "Назва додатку:");
    AddTranslation(Language::Ukrainian, "lbl_app_id", "Ідентифікатор додатку (напр. org.app.name):");
    AddTranslation(Language::Ukrainian, "lbl_version", "Версія:");
    AddTranslation(Language::Ukrainian, "lbl_author", "Автор / Організація:");
    AddTranslation(Language::Ukrainian, "lbl_description", "Опис:");
    AddTranslation(Language::Ukrainian, "lbl_category", "Категорія:");
    AddTranslation(Language::Ukrainian, "lbl_license", "Ліцензія:");
    AddTranslation(Language::Ukrainian, "lbl_install_prefix", "Типова тека встановлення:");
    AddTranslation(Language::Ukrainian, "lbl_status", "Стан:");
    AddTranslation(Language::Ukrainian, "lbl_progress", "Прогрес:");
    AddTranslation(Language::Ukrainian, "lbl_source_path", "Шлях джерела");
    AddTranslation(Language::Ukrainian, "lbl_target_path", "Шлях призначення");
    AddTranslation(Language::Ukrainian, "lbl_permissions", "Права доступу");
    AddTranslation(Language::Ukrainian, "lbl_dep_name", "Назва залежності:");
    AddTranslation(Language::Ukrainian, "lbl_dep_package", "Назва пакета Apt:");
    AddTranslation(Language::Ukrainian, "lbl_dep_check_cmd", "Команда перевірки:");
    AddTranslation(Language::Ukrainian, "lbl_dep_required", "Обов'язкова для встановлення");
    AddTranslation(Language::Ukrainian, "lbl_pre_install_js", "Скрипт перед встановленням (Pre-Install):");
    AddTranslation(Language::Ukrainian, "lbl_post_install_js", "Скрипт після встановлення (налаштування ~/.config, navi):");
    AddTranslation(Language::Ukrainian, "lbl_pre_uninstall_js", "Скрипт перед видаленням:");
    AddTranslation(Language::Ukrainian, "lbl_post_uninstall_js", "Скрипт після видалення:");
    AddTranslation(Language::Ukrainian, "lbl_desktop_name", "Назва ярлика:");
    AddTranslation(Language::Ukrainian, "lbl_desktop_exec", "Команда запуску:");
    AddTranslation(Language::Ukrainian, "lbl_desktop_icon", "Іконка:");
    AddTranslation(Language::Ukrainian, "lbl_desktop_categories", "Категорії:");
    AddTranslation(Language::Ukrainian, "lbl_desktop_terminal", "Запускати у терміналі");
    AddTranslation(Language::Ukrainian, "lbl_select_output", "Шлях до файлу інсталятора (.run):");
    AddTranslation(Language::Ukrainian, "lbl_packing", "Створення пакету інсталятора...");
    AddTranslation(Language::Ukrainian, "lbl_ready", "Готово.");
    AddTranslation(Language::Ukrainian, "lbl_installed", "Успішно встановлено!");
    AddTranslation(Language::Ukrainian, "lbl_installing", "Встановлення компонентів...");
    AddTranslation(Language::Ukrainian, "lbl_uninstalling", "Видалення компонентів...");
    AddTranslation(Language::Ukrainian, "lbl_welcome", "Ласкаво просимо до майстра встановлення.");
    AddTranslation(Language::Ukrainian, "lbl_destination", "Оберіть теку для встановлення:");
    AddTranslation(Language::Ukrainian, "lbl_space_required", "Потрібно місця:");
    AddTranslation(Language::Ukrainian, "lbl_elevation_needed", "Для цього шляху потрібні права адміністратора.");
    AddTranslation(Language::Ukrainian, "lbl_elevation_prompt", "Будь ласка, авторизуйтесь через Polkit.");
    AddTranslation(Language::Ukrainian, "lbl_deps_checking", "Перевірка залежностей системи...");
    AddTranslation(Language::Ukrainian, "lbl_deps_ok", "Всі обов'язкові залежності присутні.");
    AddTranslation(Language::Ukrainian, "lbl_deps_missing", "Деякі залежності відсутні:");
    AddTranslation(Language::Ukrainian, "lbl_install_complete", "Встановлення успішно завершено.");
    AddTranslation(Language::Ukrainian, "lbl_uninstall_complete", "Видалення успішно завершено.");
    AddTranslation(Language::Ukrainian, "lbl_launch_app", "Запустити додаток");
    AddTranslation(Language::Ukrainian, "lbl_language", "Мова");
    AddTranslation(Language::Ukrainian, "lbl_help", "Довідка");
    AddTranslation(Language::Ukrainian, "lbl_about", "Про InstallZone");
    AddTranslation(Language::Ukrainian, "lbl_about_desc", "InstallZone - Модульна система інсталяторів за стандартами JPL.\nГрафічний бекенд: OpenGL2 + Dear ImGui.\nСкрипти: Вбудований JavaScript рушій.\nПлатформа: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::Ukrainian, "lbl_sample_navi", "Вставити приклад конфігурації Navi");
    AddTranslation(Language::Ukrainian, "lbl_sample_mkdir", "Вставити приклад створення ~/.config");

    // Japanese (日本語)
    AddTranslation(Language::Japanese, "app_title_studio", "InstallZone Studio - インストーラー作成ツール");
    AddTranslation(Language::Japanese, "app_title_runner", "InstallZone - アプリケーションインストーラー");
    AddTranslation(Language::Japanese, "tab_general", "基本情報");
    AddTranslation(Language::Japanese, "tab_files", "ファイルとアセット");
    AddTranslation(Language::Japanese, "tab_dependencies", "依存パッケージ");
    AddTranslation(Language::Japanese, "tab_scripts", "JavaScriptスクリプト");
    AddTranslation(Language::Japanese, "tab_desktop", "デスクトップ統合");
    AddTranslation(Language::Japanese, "tab_pack", "インストーラーのパック");
    AddTranslation(Language::Japanese, "btn_new", "新規プロジェクト");
    AddTranslation(Language::Japanese, "btn_open", "プロジェクトを開く");
    AddTranslation(Language::Japanese, "btn_save", "プロジェクトを保存");
    AddTranslation(Language::Japanese, "btn_save_as", "名前を付けて保存...");
    AddTranslation(Language::Japanese, "btn_exit", "終了");
    AddTranslation(Language::Japanese, "btn_pack_installer", "スタンドアロンインストーラーのパック");
    AddTranslation(Language::Japanese, "btn_add_file", "ファイル追加");
    AddTranslation(Language::Japanese, "btn_add_directory", "ディレクトリ追加");
    AddTranslation(Language::Japanese, "btn_remove", "選択項目を削除");
    AddTranslation(Language::Japanese, "btn_clear", "すべてクリア");
    AddTranslation(Language::Japanese, "btn_add_dep", "依存関係を追加");
    AddTranslation(Language::Japanese, "btn_test_dep", "依存関係をテスト");
    AddTranslation(Language::Japanese, "btn_test_script", "スクリプト実行テスト (サンドボックス)");
    AddTranslation(Language::Japanese, "btn_next", "次へ >");
    AddTranslation(Language::Japanese, "btn_back", "< 戻る");
    AddTranslation(Language::Japanese, "btn_install", "インストール");
    AddTranslation(Language::Japanese, "btn_uninstall", "アンインストール");
    AddTranslation(Language::Japanese, "btn_reinstall", "再インストール");
    AddTranslation(Language::Japanese, "btn_cancel", "キャンセル");
    AddTranslation(Language::Japanese, "btn_finish", "完了");
    AddTranslation(Language::Japanese, "btn_browse", "参照...");
    AddTranslation(Language::Japanese, "btn_elevate", "Sudo / Polkit で管理者権限を取得");
    AddTranslation(Language::Japanese, "lbl_app_name", "アプリケーション名:");
    AddTranslation(Language::Japanese, "lbl_app_id", "アプリケーションID (例: org.app.name):");
    AddTranslation(Language::Japanese, "lbl_version", "バージョン:");
    AddTranslation(Language::Japanese, "lbl_author", "作成者 / ベンダー:");
    AddTranslation(Language::Japanese, "lbl_description", "説明:");
    AddTranslation(Language::Japanese, "lbl_category", "カテゴリ:");
    AddTranslation(Language::Japanese, "lbl_license", "ライセンス:");
    AddTranslation(Language::Japanese, "lbl_install_prefix", "デフォルトのインストール先:");
    AddTranslation(Language::Japanese, "lbl_status", "状態:");
    AddTranslation(Language::Japanese, "lbl_progress", "進行状況:");
    AddTranslation(Language::Japanese, "lbl_source_path", "ソースパス");
    AddTranslation(Language::Japanese, "lbl_target_path", "インストール先サブパス");
    AddTranslation(Language::Japanese, "lbl_permissions", "パーミッション");
    AddTranslation(Language::Japanese, "lbl_dep_name", "依存名:");
    AddTranslation(Language::Japanese, "lbl_dep_package", "Aptパッケージ名:");
    AddTranslation(Language::Japanese, "lbl_dep_check_cmd", "確認コマンド:");
    AddTranslation(Language::Japanese, "lbl_dep_required", "必須の依存パッケージ");
    AddTranslation(Language::Japanese, "lbl_pre_install_js", "インストール前スクリプト (Pre-Install):");
    AddTranslation(Language::Japanese, "lbl_post_install_js", "インストール後スクリプト (Post-Install, 例: ~/.config設定, navi):");
    AddTranslation(Language::Japanese, "lbl_pre_uninstall_js", "アンインストール前スクリプト:");
    AddTranslation(Language::Japanese, "lbl_post_uninstall_js", "アンインストール後スクリプト:");
    AddTranslation(Language::Japanese, "lbl_desktop_name", "デスクトップ表示名:");
    AddTranslation(Language::Japanese, "lbl_desktop_exec", "起動コマンド:");
    AddTranslation(Language::Japanese, "lbl_desktop_icon", "アイコン:");
    AddTranslation(Language::Japanese, "lbl_desktop_categories", "カテゴリ:");
    AddTranslation(Language::Japanese, "lbl_desktop_terminal", "ターミナルで実行");
    AddTranslation(Language::Japanese, "lbl_select_output", "出力ファイルパス (.run):");
    AddTranslation(Language::Japanese, "lbl_packing", "インストーラーのパッケージを作成中...");
    AddTranslation(Language::Japanese, "lbl_ready", "準備完了。");
    AddTranslation(Language::Japanese, "lbl_installed", "インストールが正常に完了しました！");
    AddTranslation(Language::Japanese, "lbl_installing", "ファイルをインストール中...");
    AddTranslation(Language::Japanese, "lbl_uninstalling", "コンポーネントを削除中...");
    AddTranslation(Language::Japanese, "lbl_welcome", "インストールウィザードへようこそ。");
    AddTranslation(Language::Japanese, "lbl_destination", "インストール先フォルダを選択:");
    AddTranslation(Language::Japanese, "lbl_space_required", "必要な空き容量:");
    AddTranslation(Language::Japanese, "lbl_elevation_needed", "このパスには管理者権限が必要です。");
    AddTranslation(Language::Japanese, "lbl_elevation_prompt", "Polkitで認証を行ってください。");
    AddTranslation(Language::Japanese, "lbl_deps_checking", "システムの依存関係を確認しています...");
    AddTranslation(Language::Japanese, "lbl_deps_ok", "すべての依存関係が満たされています。");
    AddTranslation(Language::Japanese, "lbl_deps_missing", "不足している依存関係があります:");
    AddTranslation(Language::Japanese, "lbl_install_complete", "インストールが正常に完了しました。");
    AddTranslation(Language::Japanese, "lbl_uninstall_complete", "アンインストールが正常に完了しました。");
    AddTranslation(Language::Japanese, "lbl_launch_app", "アプリケーションを起動");
    AddTranslation(Language::Japanese, "lbl_language", "言語 (Language)");
    AddTranslation(Language::Japanese, "lbl_help", "ヘルプ");
    AddTranslation(Language::Japanese, "lbl_about", "InstallZone について");
    AddTranslation(Language::Japanese, "lbl_about_desc", "InstallZone - JPL基準のモジュール式スクリプタブルインストーラーシステム。\nバックエンド: OpenGL2 + Dear ImGui\nスクリプト: 組み込みJavaScriptエンジン\nターゲット: Linux / Kubuntu 26.04 LTS (KDE Plasma)");
    AddTranslation(Language::Japanese, "lbl_sample_navi", "Navi設定スニペットを挿入");
    AddTranslation(Language::Japanese, "lbl_sample_mkdir", "~/.config作成スニペットを挿入");

    // Chinese Simplified (简体中文)
    AddTranslation(Language::Chinese, "app_title_studio", "InstallZone Studio - 安装包制作工具");
    AddTranslation(Language::Chinese, "app_title_runner", "InstallZone - 应用程序安装程序");
    AddTranslation(Language::Chinese, "tab_general", "基本信息");
    AddTranslation(Language::Chinese, "tab_files", "文件与资源");
    AddTranslation(Language::Chinese, "tab_dependencies", "依赖项管理");
    AddTranslation(Language::Chinese, "tab_scripts", "JavaScript 脚本");
    AddTranslation(Language::Chinese, "tab_desktop", "桌面集成");
    AddTranslation(Language::Chinese, "tab_pack", "打包安装程序");
    AddTranslation(Language::Chinese, "btn_new", "新建工程");
    AddTranslation(Language::Chinese, "btn_open", "打开工程");
    AddTranslation(Language::Chinese, "btn_save", "保存工程");
    AddTranslation(Language::Chinese, "btn_save_as", "另存为...");
    AddTranslation(Language::Chinese, "btn_exit", "退出");
    AddTranslation(Language::Chinese, "btn_pack_installer", "打包独立安装程序");
    AddTranslation(Language::Chinese, "btn_add_file", "添加文件");
    AddTranslation(Language::Chinese, "btn_add_directory", "添加文件夹");
    AddTranslation(Language::Chinese, "btn_remove", "移除所选");
    AddTranslation(Language::Chinese, "btn_clear", "清空所有");
    AddTranslation(Language::Chinese, "btn_add_dep", "添加依赖项");
    AddTranslation(Language::Chinese, "btn_test_dep", "检查依赖项");
    AddTranslation(Language::Chinese, "btn_test_script", "测试运行脚本 (沙箱)");
    AddTranslation(Language::Chinese, "btn_next", "下一步 >");
    AddTranslation(Language::Chinese, "btn_back", "< 上一步");
    AddTranslation(Language::Chinese, "btn_install", "立即安装");
    AddTranslation(Language::Chinese, "btn_uninstall", "卸载");
    AddTranslation(Language::Chinese, "btn_reinstall", "重新安装");
    AddTranslation(Language::Chinese, "btn_cancel", "取消");
    AddTranslation(Language::Chinese, "btn_finish", "完成");
    AddTranslation(Language::Chinese, "btn_browse", "浏览...");
    AddTranslation(Language::Chinese, "btn_elevate", "使用 Sudo / Polkit 提升权限");
    AddTranslation(Language::Chinese, "lbl_app_name", "应用程序名称:");
    AddTranslation(Language::Chinese, "lbl_app_id", "应用程序标识 (例如: org.app.name):");
    AddTranslation(Language::Chinese, "lbl_version", "版本号:");
    AddTranslation(Language::Chinese, "lbl_author", "作者 / 机构:");
    AddTranslation(Language::Chinese, "lbl_description", "描述:");
    AddTranslation(Language::Chinese, "lbl_category", "分类:");
    AddTranslation(Language::Chinese, "lbl_license", "许可证:");
    AddTranslation(Language::Chinese, "lbl_install_prefix", "默认安装路径:");
    AddTranslation(Language::Chinese, "lbl_status", "状态:");
    AddTranslation(Language::Chinese, "lbl_progress", "进度:");
    AddTranslation(Language::Chinese, "lbl_source_path", "源路径");
    AddTranslation(Language::Chinese, "lbl_target_path", "目标相对路径");
    AddTranslation(Language::Chinese, "lbl_permissions", "权限");
    AddTranslation(Language::Chinese, "lbl_dep_name", "依赖名称:");
    AddTranslation(Language::Chinese, "lbl_dep_package", "Apt 软件包名称:");
    AddTranslation(Language::Chinese, "lbl_dep_check_cmd", "检查命令:");
    AddTranslation(Language::Chinese, "lbl_dep_required", "必须依赖项");
    AddTranslation(Language::Chinese, "lbl_pre_install_js", "安装前 JavaScript 钩子 (Pre-Install):");
    AddTranslation(Language::Chinese, "lbl_post_install_js", "安装后 JavaScript 钩子 (如配置 ~/.config, navi):");
    AddTranslation(Language::Chinese, "lbl_pre_uninstall_js", "卸载前 JavaScript 钩子:");
    AddTranslation(Language::Chinese, "lbl_post_uninstall_js", "卸载后 JavaScript 钩子:");
    AddTranslation(Language::Chinese, "lbl_desktop_name", "桌面快捷方式名称:");
    AddTranslation(Language::Chinese, "lbl_desktop_exec", "启动命令:");
    AddTranslation(Language::Chinese, "lbl_desktop_icon", "图标:");
    AddTranslation(Language::Chinese, "lbl_desktop_categories", "分类:");
    AddTranslation(Language::Chinese, "lbl_desktop_terminal", "在终端中运行");
    AddTranslation(Language::Chinese, "lbl_select_output", "输出安装程序路径 (.run):");
    AddTranslation(Language::Chinese, "lbl_packing", "正在打包安装程序...");
    AddTranslation(Language::Chinese, "lbl_ready", "就绪。");
    AddTranslation(Language::Chinese, "lbl_installed", "安装成功！");
    AddTranslation(Language::Chinese, "lbl_installing", "正在安装文件...");
    AddTranslation(Language::Chinese, "lbl_uninstalling", "正在移除组件...");
    AddTranslation(Language::Chinese, "lbl_welcome", "欢迎使用安装向导。");
    AddTranslation(Language::Chinese, "lbl_destination", "选择安装目标目录:");
    AddTranslation(Language::Chinese, "lbl_space_required", "所需空间:");
    AddTranslation(Language::Chinese, "lbl_elevation_needed", "该路径需要管理员权限。");
    AddTranslation(Language::Chinese, "lbl_elevation_prompt", "请通过 Polkit 进行身份验证以继续安装。");
    AddTranslation(Language::Chinese, "lbl_deps_checking", "正在检查系统依赖项...");
    AddTranslation(Language::Chinese, "lbl_deps_ok", "所有必须的依赖项均已满足。");
    AddTranslation(Language::Chinese, "lbl_deps_missing", "缺少以下依赖项:");
    AddTranslation(Language::Chinese, "lbl_install_complete", "安装已顺利完成。");
    AddTranslation(Language::Chinese, "lbl_uninstall_complete", "卸载已顺利完成。");
    AddTranslation(Language::Chinese, "lbl_launch_app", "运行应用程序");
    AddTranslation(Language::Chinese, "lbl_language", "语言 (Language)");
    AddTranslation(Language::Chinese, "lbl_help", "帮助");
    AddTranslation(Language::Chinese, "lbl_about", "关于 InstallZone");
    AddTranslation(Language::Chinese, "lbl_about_desc", "InstallZone - 遵循 JPL 标准的高可靠模块化脚本化安装系统。\n图形后端: OpenGL2 + Dear ImGui。\n脚本: 内嵌 JavaScript 引擎。\n目标平台: Linux / Kubuntu 26.04 LTS (KDE Plasma)。");
    AddTranslation(Language::Chinese, "lbl_sample_navi", "插入 Navi 配置代码段");
    AddTranslation(Language::Chinese, "lbl_sample_mkdir", "插入 ~/.config 目录创建代码段");

    // Spanish (Español)
    AddTranslation(Language::Spanish, "app_title_studio", "InstallZone Studio - Creador de Instaladores");
    AddTranslation(Language::Spanish, "app_title_runner", "InstallZone - Instalador de Aplicaciones");
    AddTranslation(Language::Spanish, "tab_general", "Información General");
    AddTranslation(Language::Spanish, "tab_files", "Archivos y Recursos");
    AddTranslation(Language::Spanish, "tab_dependencies", "Dependencias");
    AddTranslation(Language::Spanish, "tab_scripts", "Scripts JavaScript");
    AddTranslation(Language::Spanish, "tab_desktop", "Integración de Escritorio");
    AddTranslation(Language::Spanish, "tab_pack", "Empaquetar Instalador");
    AddTranslation(Language::Spanish, "btn_new", "Nuevo Proyecto");
    AddTranslation(Language::Spanish, "btn_open", "Abrir Proyecto");
    AddTranslation(Language::Spanish, "btn_save", "Guardar Proyecto");
    AddTranslation(Language::Spanish, "btn_save_as", "Guardar Como...");
    AddTranslation(Language::Spanish, "btn_exit", "Salir");
    AddTranslation(Language::Spanish, "btn_pack_installer", "Empaquetar Instalador Autónomo");
    AddTranslation(Language::Spanish, "btn_add_file", "Añadir Archivo");
    AddTranslation(Language::Spanish, "btn_add_directory", "Añadir Directorio");
    AddTranslation(Language::Spanish, "btn_remove", "Eliminar Seleccionado");
    AddTranslation(Language::Spanish, "btn_clear", "Limpiar Todo");
    AddTranslation(Language::Spanish, "btn_add_dep", "Añadir Dependencia");
    AddTranslation(Language::Spanish, "btn_test_dep", "Comprobar Dependencia");
    AddTranslation(Language::Spanish, "btn_test_script", "Probar Script (Sandbox)");
    AddTranslation(Language::Spanish, "btn_next", "Siguiente >");
    AddTranslation(Language::Spanish, "btn_back", "< Atrás");
    AddTranslation(Language::Spanish, "btn_install", "Instalar");
    AddTranslation(Language::Spanish, "btn_uninstall", "Desinstalar");
    AddTranslation(Language::Spanish, "btn_reinstall", "Reinstalar");
    AddTranslation(Language::Spanish, "btn_cancel", "Cancelar");
    AddTranslation(Language::Spanish, "btn_finish", "Finalizar");
    AddTranslation(Language::Spanish, "btn_browse", "Examinar...");
    AddTranslation(Language::Spanish, "btn_elevate", "Elevar privilegios con Sudo / Polkit");
    AddTranslation(Language::Spanish, "lbl_app_name", "Nombre de la Aplicación:");
    AddTranslation(Language::Spanish, "lbl_app_id", "ID de Aplicación (ej. org.app.name):");
    AddTranslation(Language::Spanish, "lbl_version", "Versión:");
    AddTranslation(Language::Spanish, "lbl_author", "Autor / Proveedor:");
    AddTranslation(Language::Spanish, "lbl_description", "Descripción:");
    AddTranslation(Language::Spanish, "lbl_category", "Categoría:");
    AddTranslation(Language::Spanish, "lbl_license", "Licencia:");
    AddTranslation(Language::Spanish, "lbl_install_prefix", "Directorio de Instalación Predeterminado:");
    AddTranslation(Language::Spanish, "lbl_status", "Estado:");
    AddTranslation(Language::Spanish, "lbl_progress", "Progreso:");
    AddTranslation(Language::Spanish, "lbl_source_path", "Ruta de Origen");
    AddTranslation(Language::Spanish, "lbl_target_path", "Subruta de Destino");
    AddTranslation(Language::Spanish, "lbl_permissions", "Permisos");
    AddTranslation(Language::Spanish, "lbl_dep_name", "Nombre de Dependencia:");
    AddTranslation(Language::Spanish, "lbl_dep_package", "Paquete Apt:");
    AddTranslation(Language::Spanish, "lbl_dep_check_cmd", "Comando de Verificación:");
    AddTranslation(Language::Spanish, "lbl_dep_required", "Requerido para la Instalación");
    AddTranslation(Language::Spanish, "lbl_pre_install_js", "Script Pre-Instalación:");
    AddTranslation(Language::Spanish, "lbl_post_install_js", "Script Post-Instalación (ej. config ~/.config, navi):");
    AddTranslation(Language::Spanish, "lbl_pre_uninstall_js", "Script Pre-Desinstalación:");
    AddTranslation(Language::Spanish, "lbl_post_uninstall_js", "Script Post-Desinstalación:");
    AddTranslation(Language::Spanish, "lbl_desktop_name", "Nombre en Escritorio:");
    AddTranslation(Language::Spanish, "lbl_desktop_exec", "Comando de Ejecución:");
    AddTranslation(Language::Spanish, "lbl_desktop_icon", "Icono:");
    AddTranslation(Language::Spanish, "lbl_desktop_categories", "Categorías:");
    AddTranslation(Language::Spanish, "lbl_desktop_terminal", "Ejecutar en Terminal");
    AddTranslation(Language::Spanish, "lbl_select_output", "Binario de Salida (.run):");
    AddTranslation(Language::Spanish, "lbl_packing", "Empaquetando instalador...");
    AddTranslation(Language::Spanish, "lbl_ready", "Listo.");
    AddTranslation(Language::Spanish, "lbl_installed", "¡Instalado con éxito!");
    AddTranslation(Language::Spanish, "lbl_installing", "Instalando componentes...");
    AddTranslation(Language::Spanish, "lbl_uninstalling", "Eliminando componentes...");
    AddTranslation(Language::Spanish, "lbl_welcome", "Bienvenido al asistente de instalación.");
    AddTranslation(Language::Spanish, "lbl_destination", "Elija la carpeta de destino:");
    AddTranslation(Language::Spanish, "lbl_space_required", "Espacio necesario:");
    AddTranslation(Language::Spanish, "lbl_elevation_needed", "Se requieren permisos de administrador.");
    AddTranslation(Language::Spanish, "lbl_elevation_prompt", "Por favor autentíquese vía Polkit.");
    AddTranslation(Language::Spanish, "lbl_deps_checking", "Comprobando dependencias del sistema...");
    AddTranslation(Language::Spanish, "lbl_deps_ok", "Todas las dependencias están satisfechas.");
    AddTranslation(Language::Spanish, "lbl_deps_missing", "Faltan algunas dependencias:");
    AddTranslation(Language::Spanish, "lbl_install_complete", "La instalación ha finalizado con éxito.");
    AddTranslation(Language::Spanish, "lbl_uninstall_complete", "La desinstalación ha finalizado con éxito.");
    AddTranslation(Language::Spanish, "lbl_launch_app", "Iniciar Aplicación");
    AddTranslation(Language::Spanish, "lbl_language", "Idioma");
    AddTranslation(Language::Spanish, "lbl_help", "Ayuda");
    AddTranslation(Language::Spanish, "lbl_about", "Acerca de InstallZone");
    AddTranslation(Language::Spanish, "lbl_about_desc", "InstallZone - Sistema de instalación modular inspirado en estándares JPL.\nBackend: OpenGL2 + Dear ImGui.\nScripting: Motor JavaScript integrado.\nPlataforma: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::Spanish, "lbl_sample_navi", "Insertar fragmento de config de Navi");
    AddTranslation(Language::Spanish, "lbl_sample_mkdir", "Insertar fragmento para crear ~/.config");

    // French (Français)
    AddTranslation(Language::French, "app_title_studio", "InstallZone Studio - Créateur d'Installeurs");
    AddTranslation(Language::French, "app_title_runner", "InstallZone - Programme d'Installation");
    AddTranslation(Language::French, "tab_general", "Général");
    AddTranslation(Language::French, "tab_files", "Fichiers & Ressources");
    AddTranslation(Language::French, "tab_dependencies", "Dépendances");
    AddTranslation(Language::French, "tab_scripts", "Scripts JavaScript");
    AddTranslation(Language::French, "tab_desktop", "Intégration Bureau");
    AddTranslation(Language::French, "tab_pack", "Créer l'Installeur");
    AddTranslation(Language::French, "btn_new", "Nouveau Projet");
    AddTranslation(Language::French, "btn_open", "Ouvrir Projet");
    AddTranslation(Language::French, "btn_save", "Enregistrer");
    AddTranslation(Language::French, "btn_save_as", "Enregistrer sous...");
    AddTranslation(Language::French, "btn_exit", "Quitter");
    AddTranslation(Language::French, "btn_pack_installer", "Créer l'Installeur Autonome");
    AddTranslation(Language::French, "btn_add_file", "Ajouter un Fichier");
    AddTranslation(Language::French, "btn_add_directory", "Ajouter un Répertoire");
    AddTranslation(Language::French, "btn_remove", "Supprimer");
    AddTranslation(Language::French, "btn_clear", "Tout Effacer");
    AddTranslation(Language::French, "btn_add_dep", "Ajouter Dépendance");
    AddTranslation(Language::French, "btn_test_dep", "Vérifier Dépendance");
    AddTranslation(Language::French, "btn_test_script", "Tester le Script (Sandbox)");
    AddTranslation(Language::French, "btn_next", "Suivant >");
    AddTranslation(Language::French, "btn_back", "< Précédent");
    AddTranslation(Language::French, "btn_install", "Installer");
    AddTranslation(Language::French, "btn_uninstall", "Désinstaller");
    AddTranslation(Language::French, "btn_reinstall", "Réinstaller");
    AddTranslation(Language::French, "btn_cancel", "Annuler");
    AddTranslation(Language::French, "btn_finish", "Terminer");
    AddTranslation(Language::French, "btn_browse", "Parcourir...");
    AddTranslation(Language::French, "btn_elevate", "Élever les privilèges (Sudo / Polkit)");
    AddTranslation(Language::French, "lbl_app_name", "Nom de l'application :");
    AddTranslation(Language::French, "lbl_app_id", "ID de l'application (ex. org.app.name) :");
    AddTranslation(Language::French, "lbl_version", "Version :");
    AddTranslation(Language::French, "lbl_author", "Auteur / Fournisseur :");
    AddTranslation(Language::French, "lbl_description", "Description :");
    AddTranslation(Language::French, "lbl_category", "Catégorie :");
    AddTranslation(Language::French, "lbl_license", "Licence :");
    AddTranslation(Language::French, "lbl_install_prefix", "Répertoire d'installation par défaut :");
    AddTranslation(Language::French, "lbl_status", "Statut :");
    AddTranslation(Language::French, "lbl_progress", "Progression :");
    AddTranslation(Language::French, "lbl_source_path", "Chemin Source");
    AddTranslation(Language::French, "lbl_target_path", "Chemin Cible");
    AddTranslation(Language::French, "lbl_permissions", "Permissions");
    AddTranslation(Language::French, "lbl_dep_name", "Nom de la dépendance :");
    AddTranslation(Language::French, "lbl_dep_package", "Nom du paquet Apt :");
    AddTranslation(Language::French, "lbl_dep_check_cmd", "Commande de vérification :");
    AddTranslation(Language::French, "lbl_dep_required", "Requis pour l'installation");
    AddTranslation(Language::French, "lbl_pre_install_js", "Script Pré-Installation :");
    AddTranslation(Language::French, "lbl_post_install_js", "Script Post-Installation (ex. ~/.config, navi) :");
    AddTranslation(Language::French, "lbl_pre_uninstall_js", "Script Pré-Désinstallation :");
    AddTranslation(Language::French, "lbl_post_uninstall_js", "Script Post-Désinstallation :");
    AddTranslation(Language::French, "lbl_desktop_name", "Nom de l'icône bureau :");
    AddTranslation(Language::French, "lbl_desktop_exec", "Commande d'exécution :");
    AddTranslation(Language::French, "lbl_desktop_icon", "Icône :");
    AddTranslation(Language::French, "lbl_desktop_categories", "Catégories :");
    AddTranslation(Language::French, "lbl_desktop_terminal", "Lancer dans le terminal");
    AddTranslation(Language::French, "lbl_select_output", "Fichier d'installation de sortie (.run) :");
    AddTranslation(Language::French, "lbl_packing", "Création du paquet...");
    AddTranslation(Language::French, "lbl_ready", "Prêt.");
    AddTranslation(Language::French, "lbl_installed", "Installé avec succès !");
    AddTranslation(Language::French, "lbl_installing", "Installation des composants...");
    AddTranslation(Language::French, "lbl_uninstalling", "Suppression des composants...");
    AddTranslation(Language::French, "lbl_welcome", "Bienvenue dans l'assistant d'installation.");
    AddTranslation(Language::French, "lbl_destination", "Sélectionnez le dossier de destination :");
    AddTranslation(Language::French, "lbl_space_required", "Espace requis :");
    AddTranslation(Language::French, "lbl_elevation_needed", "Privilèges administrateur requis pour ce chemin.");
    AddTranslation(Language::French, "lbl_elevation_prompt", "Veuillez vous authentifier avec Polkit.");
    AddTranslation(Language::French, "lbl_deps_checking", "Vérification des dépendances système...");
    AddTranslation(Language::French, "lbl_deps_ok", "Toutes les dépendances requises sont présentes.");
    AddTranslation(Language::French, "lbl_deps_missing", "Certaines dépendances sont manquantes :");
    AddTranslation(Language::French, "lbl_install_complete", "L'installation s'est terminée avec succès.");
    AddTranslation(Language::French, "lbl_uninstall_complete", "La désinstallation s'est terminée avec succès.");
    AddTranslation(Language::French, "lbl_launch_app", "Lancer l'application");
    AddTranslation(Language::French, "lbl_language", "Langue");
    AddTranslation(Language::French, "lbl_help", "Aide");
    AddTranslation(Language::French, "lbl_about", "À propos d'InstallZone");
    AddTranslation(Language::French, "lbl_about_desc", "InstallZone - Système d'installation scriptable et modulaire respectant les standards JPL.\nBackend: OpenGL2 + Dear ImGui.\nScripting: Moteur JavaScript intégré.\nPlateforme: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::French, "lbl_sample_navi", "Insérer modèle config Navi");
    AddTranslation(Language::French, "lbl_sample_mkdir", "Insérer modèle création ~/.config");

    // German (Deutsch)
    AddTranslation(Language::German, "app_title_studio", "InstallZone Studio - Installer-Erstellung");
    AddTranslation(Language::German, "app_title_runner", "InstallZone - Anwendungs-Installer");
    AddTranslation(Language::German, "tab_general", "Allgemein");
    AddTranslation(Language::German, "tab_files", "Dateien & Assets");
    AddTranslation(Language::German, "tab_dependencies", "Abhängigkeiten");
    AddTranslation(Language::German, "tab_scripts", "JavaScript-Skripte");
    AddTranslation(Language::German, "tab_desktop", "Desktop-Integration");
    AddTranslation(Language::German, "tab_pack", "Installer packen");
    AddTranslation(Language::German, "btn_new", "Neues Projekt");
    AddTranslation(Language::German, "btn_open", "Projekt öffnen");
    AddTranslation(Language::German, "btn_save", "Projekt speichern");
    AddTranslation(Language::German, "btn_save_as", "Speichern unter...");
    AddTranslation(Language::German, "btn_exit", "Beenden");
    AddTranslation(Language::German, "btn_pack_installer", "Eigenständigen Installer packen");
    AddTranslation(Language::German, "btn_add_file", "Datei hinzufügen");
    AddTranslation(Language::German, "btn_add_directory", "Verzeichnis hinzufügen");
    AddTranslation(Language::German, "btn_remove", "Auswahl entfernen");
    AddTranslation(Language::German, "btn_clear", "Alles löschen");
    AddTranslation(Language::German, "btn_add_dep", "Abhängigkeit hinzufügen");
    AddTranslation(Language::German, "btn_test_dep", "Abhängigkeit prüfen");
    AddTranslation(Language::German, "btn_test_script", "Skript testen (Sandbox)");
    AddTranslation(Language::German, "btn_next", "Weiter >");
    AddTranslation(Language::German, "btn_back", "< Zurück");
    AddTranslation(Language::German, "btn_install", "Installieren");
    AddTranslation(Language::German, "btn_uninstall", "Deinstallieren");
    AddTranslation(Language::German, "btn_reinstall", "Neu installieren");
    AddTranslation(Language::German, "btn_cancel", "Abbrechen");
    AddTranslation(Language::German, "btn_finish", "Fertigstellen");
    AddTranslation(Language::German, "btn_browse", "Durchsuchen...");
    AddTranslation(Language::German, "btn_elevate", "Admin-Rechte anfordern (Sudo / Polkit)");
    AddTranslation(Language::German, "lbl_app_name", "Anwendungsname:");
    AddTranslation(Language::German, "lbl_app_id", "Anwendungs-ID (z.B. org.app.name):");
    AddTranslation(Language::German, "lbl_version", "Version:");
    AddTranslation(Language::German, "lbl_author", "Autor / Anbieter:");
    AddTranslation(Language::German, "lbl_description", "Beschreibung:");
    AddTranslation(Language::German, "lbl_category", "Kategorie:");
    AddTranslation(Language::German, "lbl_license", "Lizenz:");
    AddTranslation(Language::German, "lbl_install_prefix", "Standard-Installationspfad:");
    AddTranslation(Language::German, "lbl_status", "Status:");
    AddTranslation(Language::German, "lbl_progress", "Fortschritt:");
    AddTranslation(Language::German, "lbl_source_path", "Quellpfad");
    AddTranslation(Language::German, "lbl_target_path", "Ziel-Unterpfad");
    AddTranslation(Language::German, "lbl_permissions", "Berechtigungen");
    AddTranslation(Language::German, "lbl_dep_name", "Name der Abhängigkeit:");
    AddTranslation(Language::German, "lbl_dep_package", "Apt-Paketname:");
    AddTranslation(Language::German, "lbl_dep_check_cmd", "Prüfbefehl:");
    AddTranslation(Language::German, "lbl_dep_required", "Erforderlich für Installation");
    AddTranslation(Language::German, "lbl_pre_install_js", "Vor-Installations-Skript:");
    AddTranslation(Language::German, "lbl_post_install_js", "Nach-Installations-Skript (z.B. ~/.config, navi):");
    AddTranslation(Language::German, "lbl_pre_uninstall_js", "Vor-Deinstallations-Skript:");
    AddTranslation(Language::German, "lbl_post_uninstall_js", "Nach-Deinstallations-Skript:");
    AddTranslation(Language::German, "lbl_desktop_name", "Name im Anwendungsmenü:");
    AddTranslation(Language::German, "lbl_desktop_exec", "Startbefehl:");
    AddTranslation(Language::German, "lbl_desktop_icon", "Symbol:");
    AddTranslation(Language::German, "lbl_desktop_categories", "Kategorien:");
    AddTranslation(Language::German, "lbl_desktop_terminal", "Im Terminal ausführen");
    AddTranslation(Language::German, "lbl_select_output", "Ausgabe-Installer-Pfad (.run):");
    AddTranslation(Language::German, "lbl_packing", "Installer wird gepackt...");
    AddTranslation(Language::German, "lbl_ready", "Bereit.");
    AddTranslation(Language::German, "lbl_installed", "Erfolgreich installiert!");
    AddTranslation(Language::German, "lbl_installing", "Dateien werden installiert...");
    AddTranslation(Language::German, "lbl_uninstalling", "Komponenten werden entfernt...");
    AddTranslation(Language::German, "lbl_welcome", "Willkommen beim Installationsassistenten.");
    AddTranslation(Language::German, "lbl_destination", "Zielordner auswählen:");
    AddTranslation(Language::German, "lbl_space_required", "Benötigter Speicherplatz:");
    AddTranslation(Language::German, "lbl_elevation_needed", "Für diesen Pfad sind Administratorrechte erforderlich.");
    AddTranslation(Language::German, "lbl_elevation_prompt", "Bitte über Polkit authentifizieren.");
    AddTranslation(Language::German, "lbl_deps_checking", "Systemabhängigkeiten werden überprüft...");
    AddTranslation(Language::German, "lbl_deps_ok", "Alle erforderlichen Abhängigkeiten sind vorhanden.");
    AddTranslation(Language::German, "lbl_deps_missing", "Folgende Abhängigkeiten fehlen:");
    AddTranslation(Language::German, "lbl_install_complete", "Installation erfolgreich abgeschlossen.");
    AddTranslation(Language::German, "lbl_uninstall_complete", "Deinstallation erfolgreich abgeschlossen.");
    AddTranslation(Language::German, "lbl_launch_app", "Anwendung starten");
    AddTranslation(Language::German, "lbl_language", "Sprache");
    AddTranslation(Language::German, "lbl_help", "Hilfe");
    AddTranslation(Language::German, "lbl_about", "Über InstallZone");
    AddTranslation(Language::German, "lbl_about_desc", "InstallZone - Modulares, JPL-inspiriertes Installationssystem.\nBackend: OpenGL2 + Dear ImGui.\nSkripting: Integrierte JavaScript-Engine.\nZiel: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::German, "lbl_sample_navi", "Navi-Konfigurationsvorlage einfügen");
    AddTranslation(Language::German, "lbl_sample_mkdir", "~/.config-Erstellungsvorlage einfügen");

    // Portuguese (Português)
    AddTranslation(Language::Portuguese, "app_title_studio", "InstallZone Studio - Criador de Instaladores");
    AddTranslation(Language::Portuguese, "app_title_runner", "InstallZone - Instalador de Aplicativos");
    AddTranslation(Language::Portuguese, "tab_general", "Geral");
    AddTranslation(Language::Portuguese, "tab_files", "Arquivos e Recursos");
    AddTranslation(Language::Portuguese, "tab_dependencies", "Dependências");
    AddTranslation(Language::Portuguese, "tab_scripts", "Scripts JavaScript");
    AddTranslation(Language::Portuguese, "tab_desktop", "Integração Desktop");
    AddTranslation(Language::Portuguese, "tab_pack", "Empacotar Instalador");
    AddTranslation(Language::Portuguese, "btn_new", "Novo Projeto");
    AddTranslation(Language::Portuguese, "btn_open", "Abrir Projeto");
    AddTranslation(Language::Portuguese, "btn_save", "Salvar Projeto");
    AddTranslation(Language::Portuguese, "btn_save_as", "Salvar Como...");
    AddTranslation(Language::Portuguese, "btn_exit", "Sair");
    AddTranslation(Language::Portuguese, "btn_pack_installer", "Empacotar Instalador Autônomo");
    AddTranslation(Language::Portuguese, "btn_add_file", "Adicionar Arquivo");
    AddTranslation(Language::Portuguese, "btn_add_directory", "Adicionar Pasta");
    AddTranslation(Language::Portuguese, "btn_remove", "Remover");
    AddTranslation(Language::Portuguese, "btn_clear", "Limpar Tudo");
    AddTranslation(Language::Portuguese, "btn_add_dep", "Adicionar Dependência");
    AddTranslation(Language::Portuguese, "btn_test_dep", "Verificar Dependência");
    AddTranslation(Language::Portuguese, "btn_test_script", "Testar Script (Sandbox)");
    AddTranslation(Language::Portuguese, "btn_next", "Avançar >");
    AddTranslation(Language::Portuguese, "btn_back", "< Voltar");
    AddTranslation(Language::Portuguese, "btn_install", "Instalar");
    AddTranslation(Language::Portuguese, "btn_uninstall", "Desinstalar");
    AddTranslation(Language::Portuguese, "btn_reinstall", "Reinstalar");
    AddTranslation(Language::Portuguese, "btn_cancel", "Cancelar");
    AddTranslation(Language::Portuguese, "btn_finish", "Concluir");
    AddTranslation(Language::Portuguese, "btn_browse", "Procurar...");
    AddTranslation(Language::Portuguese, "btn_elevate", "Elevar Privilégios com Sudo / Polkit");
    AddTranslation(Language::Portuguese, "lbl_app_name", "Nome do Aplicativo:");
    AddTranslation(Language::Portuguese, "lbl_app_id", "ID do Aplicativo (ex: org.app.name):");
    AddTranslation(Language::Portuguese, "lbl_version", "Versão:");
    AddTranslation(Language::Portuguese, "lbl_author", "Autor / Empresa:");
    AddTranslation(Language::Portuguese, "lbl_description", "Descrição:");
    AddTranslation(Language::Portuguese, "lbl_category", "Categoria:");
    AddTranslation(Language::Portuguese, "lbl_license", "Licença:");
    AddTranslation(Language::Portuguese, "lbl_install_prefix", "Diretório de Instalação Padrão:");
    AddTranslation(Language::Portuguese, "lbl_status", "Status:");
    AddTranslation(Language::Portuguese, "lbl_progress", "Progresso:");
    AddTranslation(Language::Portuguese, "lbl_source_path", "Caminho de Origem");
    AddTranslation(Language::Portuguese, "lbl_target_path", "Subcaminho de Destino");
    AddTranslation(Language::Portuguese, "lbl_permissions", "Permissões");
    AddTranslation(Language::Portuguese, "lbl_dep_name", "Nome da Dependência:");
    AddTranslation(Language::Portuguese, "lbl_dep_package", "Nome do Pacote Apt:");
    AddTranslation(Language::Portuguese, "lbl_dep_check_cmd", "Comando de Verificação:");
    AddTranslation(Language::Portuguese, "lbl_dep_required", "Obrigatório para Instalação");
    AddTranslation(Language::Portuguese, "lbl_pre_install_js", "Script Pré-Instalação:");
    AddTranslation(Language::Portuguese, "lbl_post_install_js", "Script Pós-Instalação (ex: ~/.config, navi):");
    AddTranslation(Language::Portuguese, "lbl_pre_uninstall_js", "Script Pré-Desinstalação:");
    AddTranslation(Language::Portuguese, "lbl_post_uninstall_js", "Script Pós-Desinstalação:");
    AddTranslation(Language::Portuguese, "lbl_desktop_name", "Nome no Menu:");
    AddTranslation(Language::Portuguese, "lbl_desktop_exec", "Comando de Execução:");
    AddTranslation(Language::Portuguese, "lbl_desktop_icon", "Ícone:");
    AddTranslation(Language::Portuguese, "lbl_desktop_categories", "Categorias:");
    AddTranslation(Language::Portuguese, "lbl_desktop_terminal", "Executar no Terminal");
    AddTranslation(Language::Portuguese, "lbl_select_output", "Arquivo do Instalador (.run):");
    AddTranslation(Language::Portuguese, "lbl_packing", "Empacotando instalador...");
    AddTranslation(Language::Portuguese, "lbl_ready", "Pronto.");
    AddTranslation(Language::Portuguese, "lbl_installed", "Instalado com sucesso!");
    AddTranslation(Language::Portuguese, "lbl_installing", "Instalando componentes...");
    AddTranslation(Language::Portuguese, "lbl_uninstalling", "Removendo componentes...");
    AddTranslation(Language::Portuguese, "lbl_welcome", "Bem-vindo ao assistente de instalação.");
    AddTranslation(Language::Portuguese, "lbl_destination", "Escolha a pasta de destino:");
    AddTranslation(Language::Portuguese, "lbl_space_required", "Espaço Necessário:");
    AddTranslation(Language::Portuguese, "lbl_elevation_needed", "Privilégios de administrador são necessários para este local.");
    AddTranslation(Language::Portuguese, "lbl_elevation_prompt", "Por favor autentique-se via Polkit.");
    AddTranslation(Language::Portuguese, "lbl_deps_checking", "Verificando dependências do sistema...");
    AddTranslation(Language::Portuguese, "lbl_deps_ok", "Todas as dependências necessárias estão atendidas.");
    AddTranslation(Language::Portuguese, "lbl_deps_missing", "Algumas dependências estão faltando:");
    AddTranslation(Language::Portuguese, "lbl_install_complete", "Instalação concluída com sucesso.");
    AddTranslation(Language::Portuguese, "lbl_uninstall_complete", "Desinstalação concluída com sucesso.");
    AddTranslation(Language::Portuguese, "lbl_launch_app", "Iniciar Aplicativo");
    AddTranslation(Language::Portuguese, "lbl_language", "Idioma");
    AddTranslation(Language::Portuguese, "lbl_help", "Ajuda");
    AddTranslation(Language::Portuguese, "lbl_about", "Sobre o InstallZone");
    AddTranslation(Language::Portuguese, "lbl_about_desc", "InstallZone - Sistema modular e seguro de instaladores inspirado no padrão JPL.\nBackend: OpenGL2 + Dear ImGui.\nScripting: Motor JavaScript integrado.\nPlataforma: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::Portuguese, "lbl_sample_navi", "Inserir modelo de configuração do Navi");
    AddTranslation(Language::Portuguese, "lbl_sample_mkdir", "Inserir modelo de criação de ~/.config");

    // Italian (Italiano)
    AddTranslation(Language::Italian, "app_title_studio", "InstallZone Studio - Creazione Installatori");
    AddTranslation(Language::Italian, "app_title_runner", "InstallZone - Installatore Applicazioni");
    AddTranslation(Language::Italian, "tab_general", "Generale");
    AddTranslation(Language::Italian, "tab_files", "File e Risorse");
    AddTranslation(Language::Italian, "tab_dependencies", "Dipendenze");
    AddTranslation(Language::Italian, "tab_scripts", "Script JavaScript");
    AddTranslation(Language::Italian, "tab_desktop", "Integrazione Desktop");
    AddTranslation(Language::Italian, "tab_pack", "Pacchetto Installatore");
    AddTranslation(Language::Italian, "btn_new", "Nuovo Progetto");
    AddTranslation(Language::Italian, "btn_open", "Apri Progetto");
    AddTranslation(Language::Italian, "btn_save", "Salva Progetto");
    AddTranslation(Language::Italian, "btn_save_as", "Salva con nome...");
    AddTranslation(Language::Italian, "btn_exit", "Esci");
    AddTranslation(Language::Italian, "btn_pack_installer", "Crea Installatore Standalone");
    AddTranslation(Language::Italian, "btn_add_file", "Aggiungi File");
    AddTranslation(Language::Italian, "btn_add_directory", "Aggiungi Cartella");
    AddTranslation(Language::Italian, "btn_remove", "Rimuovi");
    AddTranslation(Language::Italian, "btn_clear", "Pulisci Tutto");
    AddTranslation(Language::Italian, "btn_add_dep", "Aggiungi Dipendenza");
    AddTranslation(Language::Italian, "btn_test_dep", "Verifica Dipendenza");
    AddTranslation(Language::Italian, "btn_test_script", "Esegui Test Script (Sandbox)");
    AddTranslation(Language::Italian, "btn_next", "Avanti >");
    AddTranslation(Language::Italian, "btn_back", "< Indietro");
    AddTranslation(Language::Italian, "btn_install", "Installa");
    AddTranslation(Language::Italian, "btn_uninstall", "Disinstalla");
    AddTranslation(Language::Italian, "btn_reinstall", "Reinstalla");
    AddTranslation(Language::Italian, "btn_cancel", "Annulla");
    AddTranslation(Language::Italian, "btn_finish", "Fine");
    AddTranslation(Language::Italian, "btn_browse", "Sfoglia...");
    AddTranslation(Language::Italian, "btn_elevate", "Eleva privilegi con Sudo / Polkit");
    AddTranslation(Language::Italian, "lbl_app_name", "Nome Applicazione:");
    AddTranslation(Language::Italian, "lbl_app_id", "ID Applicazione (es. org.app.name):");
    AddTranslation(Language::Italian, "lbl_version", "Versione:");
    AddTranslation(Language::Italian, "lbl_author", "Autore / Fornitore:");
    AddTranslation(Language::Italian, "lbl_description", "Descrizione:");
    AddTranslation(Language::Italian, "lbl_category", "Categoria:");
    AddTranslation(Language::Italian, "lbl_license", "Licenza:");
    AddTranslation(Language::Italian, "lbl_install_prefix", "Percorso di Installazione Predefinito:");
    AddTranslation(Language::Italian, "lbl_status", "Stato:");
    AddTranslation(Language::Italian, "lbl_progress", "Avanzamento:");
    AddTranslation(Language::Italian, "lbl_source_path", "Percorso Origine");
    AddTranslation(Language::Italian, "lbl_target_path", "Sotto-percorso Destinazione");
    AddTranslation(Language::Italian, "lbl_permissions", "Permessi");
    AddTranslation(Language::Italian, "lbl_dep_name", "Nome Dipendenza:");
    AddTranslation(Language::Italian, "lbl_dep_package", "Nome Pacchetto Apt:");
    AddTranslation(Language::Italian, "lbl_dep_check_cmd", "Comando di Verifica:");
    AddTranslation(Language::Italian, "lbl_dep_required", "Richiesta per Installazione");
    AddTranslation(Language::Italian, "lbl_pre_install_js", "Script Pre-Installazione:");
    AddTranslation(Language::Italian, "lbl_post_install_js", "Script Post-Installazione (es. configurazione ~/.config, navi):");
    AddTranslation(Language::Italian, "lbl_pre_uninstall_js", "Script Pre-Disinstallazione:");
    AddTranslation(Language::Italian, "lbl_post_uninstall_js", "Script Post-Disinstallazione:");
    AddTranslation(Language::Italian, "lbl_desktop_name", "Nome nel Menu:");
    AddTranslation(Language::Italian, "lbl_desktop_exec", "Comando di Avvio:");
    AddTranslation(Language::Italian, "lbl_desktop_icon", "Icona:");
    AddTranslation(Language::Italian, "lbl_desktop_categories", "Categorie:");
    AddTranslation(Language::Italian, "lbl_desktop_terminal", "Esegui nel Terminale");
    AddTranslation(Language::Italian, "lbl_select_output", "File Installatore Output (.run):");
    AddTranslation(Language::Italian, "lbl_packing", "Creazione pacchetto...");
    AddTranslation(Language::Italian, "lbl_ready", "Pronto.");
    AddTranslation(Language::Italian, "lbl_installed", "Installazione completata con successo!");
    AddTranslation(Language::Italian, "lbl_installing", "Installazione componenti...");
    AddTranslation(Language::Italian, "lbl_uninstalling", "Rimozione componenti...");
    AddTranslation(Language::Italian, "lbl_welcome", "Benvenuto nella procedura guidata di installazione.");
    AddTranslation(Language::Italian, "lbl_destination", "Scegli la cartella di destinazione:");
    AddTranslation(Language::Italian, "lbl_space_required", "Spazio Richiesto:");
    AddTranslation(Language::Italian, "lbl_elevation_needed", "Privilegi di amministratore richiesti per questo percorso.");
    AddTranslation(Language::Italian, "lbl_elevation_prompt", "Autenticarsi tramite Polkit per continuare.");
    AddTranslation(Language::Italian, "lbl_deps_checking", "Verifica dipendenze in corso...");
    AddTranslation(Language::Italian, "lbl_deps_ok", "Tutte le dipendenze richieste sono soddisfatte.");
    AddTranslation(Language::Italian, "lbl_deps_missing", "Mancano alcune dipendenze:");
    AddTranslation(Language::Italian, "lbl_install_complete", "Installazione completata.");
    AddTranslation(Language::Italian, "lbl_uninstall_complete", "Disinstallazione completata.");
    AddTranslation(Language::Italian, "lbl_launch_app", "Avvia Applicazione");
    AddTranslation(Language::Italian, "lbl_language", "Lingua");
    AddTranslation(Language::Italian, "lbl_help", "Aiuto");
    AddTranslation(Language::Italian, "lbl_about", "Informazioni su InstallZone");
    AddTranslation(Language::Italian, "lbl_about_desc", "InstallZone - Sistema modulare per installatori conforme agli standard JPL.\nBackend: OpenGL2 + Dear ImGui.\nScripting: Motore JavaScript integrato.\nPiattaforma: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::Italian, "lbl_sample_navi", "Inserisci snippet configurazione Navi");
    AddTranslation(Language::Italian, "lbl_sample_mkdir", "Inserisci snippet creazione ~/.config");

    // Polish (Polski)
    AddTranslation(Language::Polish, "app_title_studio", "InstallZone Studio - Kreator Instalatorów");
    AddTranslation(Language::Polish, "app_title_runner", "InstallZone - Instalator Aplikacji");
    AddTranslation(Language::Polish, "tab_general", "Ogólne");
    AddTranslation(Language::Polish, "tab_files", "Pliki i Zasoby");
    AddTranslation(Language::Polish, "tab_dependencies", "Zależności");
    AddTranslation(Language::Polish, "tab_scripts", "Skrypty JavaScript");
    AddTranslation(Language::Polish, "tab_desktop", "Integracja z Pulpitem");
    AddTranslation(Language::Polish, "tab_pack", "Pakowanie Instalatora");
    AddTranslation(Language::Polish, "btn_new", "Nowy Projekt");
    AddTranslation(Language::Polish, "btn_open", "Otwórz Projekt");
    AddTranslation(Language::Polish, "btn_save", "Zapisz Projekt");
    AddTranslation(Language::Polish, "btn_save_as", "Zapisz Jako...");
    AddTranslation(Language::Polish, "btn_exit", "Wyjście");
    AddTranslation(Language::Polish, "btn_pack_installer", "Spakuj Samodzielny Instalator");
    AddTranslation(Language::Polish, "btn_add_file", "Dodaj Plik");
    AddTranslation(Language::Polish, "btn_add_directory", "Dodaj Folder");
    AddTranslation(Language::Polish, "btn_remove", "Usuń Zaznaczone");
    AddTranslation(Language::Polish, "btn_clear", "Wyczyść Wszystko");
    AddTranslation(Language::Polish, "btn_add_dep", "Dodaj Zależność");
    AddTranslation(Language::Polish, "btn_test_dep", "Sprawdź Zależność");
    AddTranslation(Language::Polish, "btn_test_script", "Przetestuj Skrypt (Sandbox)");
    AddTranslation(Language::Polish, "btn_next", "Dalej >");
    AddTranslation(Language::Polish, "btn_back", "< Wstecz");
    AddTranslation(Language::Polish, "btn_install", "Zainstaluj");
    AddTranslation(Language::Polish, "btn_uninstall", "Odinstaluj");
    AddTranslation(Language::Polish, "btn_reinstall", "Przeinstaluj");
    AddTranslation(Language::Polish, "btn_cancel", "Anuluj");
    AddTranslation(Language::Polish, "btn_finish", "Zakończ");
    AddTranslation(Language::Polish, "btn_browse", "Przeglądaj...");
    AddTranslation(Language::Polish, "btn_elevate", "Uzyskaj Uprawnienia Administratora (Polkit)");
    AddTranslation(Language::Polish, "lbl_app_name", "Nazwa Aplikacji:");
    AddTranslation(Language::Polish, "lbl_app_id", "Identyfikator (np. org.app.name):");
    AddTranslation(Language::Polish, "lbl_version", "Wersja:");
    AddTranslation(Language::Polish, "lbl_author", "Autor / Dostawca:");
    AddTranslation(Language::Polish, "lbl_description", "Opis:");
    AddTranslation(Language::Polish, "lbl_category", "Kategoria:");
    AddTranslation(Language::Polish, "lbl_license", "Licencja:");
    AddTranslation(Language::Polish, "lbl_install_prefix", "Domyślny Folder Instalacji:");
    AddTranslation(Language::Polish, "lbl_status", "Status:");
    AddTranslation(Language::Polish, "lbl_progress", "Postęp:");
    AddTranslation(Language::Polish, "lbl_source_path", "Ścieżka Źródłowa");
    AddTranslation(Language::Polish, "lbl_target_path", "Podścieżka Docelowa");
    AddTranslation(Language::Polish, "lbl_permissions", "Uprawnienia");
    AddTranslation(Language::Polish, "lbl_dep_name", "Nazwa Zależności:");
    AddTranslation(Language::Polish, "lbl_dep_package", "Pakiet Apt:");
    AddTranslation(Language::Polish, "lbl_dep_check_cmd", "Polecenie Sprawdzające:");
    AddTranslation(Language::Polish, "lbl_dep_required", "Wymagana do Instalacji");
    AddTranslation(Language::Polish, "lbl_pre_install_js", "Skrypt Przed Instalacją:");
    AddTranslation(Language::Polish, "lbl_post_install_js", "Skrypt Po Instalacji (np. ~/.config, navi):");
    AddTranslation(Language::Polish, "lbl_pre_uninstall_js", "Skrypt Przed Odinstalowaniem:");
    AddTranslation(Language::Polish, "lbl_post_uninstall_js", "Skrypt Po Odinstalowaniu:");
    AddTranslation(Language::Polish, "lbl_desktop_name", "Nazwa w Menu:");
    AddTranslation(Language::Polish, "lbl_desktop_exec", "Polecenie Uruchomienia:");
    AddTranslation(Language::Polish, "lbl_desktop_icon", "Ikona:");
    AddTranslation(Language::Polish, "lbl_desktop_categories", "Kategorie:");
    AddTranslation(Language::Polish, "lbl_desktop_terminal", "Uruchom w Terminalu");
    AddTranslation(Language::Polish, "lbl_select_output", "Ścieżka Wyjściowa Instalatora (.run):");
    AddTranslation(Language::Polish, "lbl_packing", "Pakowanie instalatora...");
    AddTranslation(Language::Polish, "lbl_ready", "Gotowy.");
    AddTranslation(Language::Polish, "lbl_installed", "Zainstalowano pomyślnie!");
    AddTranslation(Language::Polish, "lbl_installing", "Instalowanie składników...");
    AddTranslation(Language::Polish, "lbl_uninstalling", "Usuwanie składników...");
    AddTranslation(Language::Polish, "lbl_welcome", "Witamy w kreatorze instalacji.");
    AddTranslation(Language::Polish, "lbl_destination", "Wybierz folder docelowy:");
    AddTranslation(Language::Polish, "lbl_space_required", "Wymagane Miejsce:");
    AddTranslation(Language::Polish, "lbl_elevation_needed", "Uprawnienia administratora są wymagane dla tej ścieżki.");
    AddTranslation(Language::Polish, "lbl_elevation_prompt", "Zaloguj się przez Polkit, aby kontynuować.");
    AddTranslation(Language::Polish, "lbl_deps_checking", "Sprawdzanie zależności systemowych...");
    AddTranslation(Language::Polish, "lbl_deps_ok", "Wszystkie wymagane zależności są spełnione.");
    AddTranslation(Language::Polish, "lbl_deps_missing", "Brakuje niektórych zależności:");
    AddTranslation(Language::Polish, "lbl_install_complete", "Instalacja zakończona sukcesem.");
    AddTranslation(Language::Polish, "lbl_uninstall_complete", "Odinstalowanie zakończone sukcesem.");
    AddTranslation(Language::Polish, "lbl_launch_app", "Uruchom Aplikację");
    AddTranslation(Language::Polish, "lbl_language", "Język");
    AddTranslation(Language::Polish, "lbl_help", "Pomoc");
    AddTranslation(Language::Polish, "lbl_about", "O InstallZone");
    AddTranslation(Language::Polish, "lbl_about_desc", "InstallZone - Modułowy system instalacyjny zgodny ze standardami JPL.\nBackend: OpenGL2 + Dear ImGui.\nSkrypty: Wbudowany silnik JavaScript.\nPlatforma: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::Polish, "lbl_sample_navi", "Wstaw konfigurację Navi");
    AddTranslation(Language::Polish, "lbl_sample_mkdir", "Wstaw tworzenie folderu ~/.config");

    // Korean (한국어)
    AddTranslation(Language::Korean, "app_title_studio", "InstallZone Studio - 설치 프로그램 제작 도구");
    AddTranslation(Language::Korean, "app_title_runner", "InstallZone - 응용 프로그램 설치 프로그램");
    AddTranslation(Language::Korean, "tab_general", "일반 정보");
    AddTranslation(Language::Korean, "tab_files", "파일 및 리소스");
    AddTranslation(Language::Korean, "tab_dependencies", "종속성");
    AddTranslation(Language::Korean, "tab_scripts", "JavaScript 스크립트");
    AddTranslation(Language::Korean, "tab_desktop", "데스크톱 통합");
    AddTranslation(Language::Korean, "tab_pack", "설치 프로그램 패키징");
    AddTranslation(Language::Korean, "btn_new", "새 프로젝트");
    AddTranslation(Language::Korean, "btn_open", "프로젝트 열기");
    AddTranslation(Language::Korean, "btn_save", "프로젝트 저장");
    AddTranslation(Language::Korean, "btn_save_as", "다른 이름으로 저장...");
    AddTranslation(Language::Korean, "btn_exit", "종료");
    AddTranslation(Language::Korean, "btn_pack_installer", "독립 실행형 설치 프로그램 패키징");
    AddTranslation(Language::Korean, "btn_add_file", "파일 추가");
    AddTranslation(Language::Korean, "btn_add_directory", "디렉터리 추가");
    AddTranslation(Language::Korean, "btn_remove", "선택 항목 삭제");
    AddTranslation(Language::Korean, "btn_clear", "모두 지우기");
    AddTranslation(Language::Korean, "btn_add_dep", "종속성 추가");
    AddTranslation(Language::Korean, "btn_test_dep", "종속성 확인");
    AddTranslation(Language::Korean, "btn_test_script", "스크립트 테스트 실행 (샌드박스)");
    AddTranslation(Language::Korean, "btn_next", "다음 >");
    AddTranslation(Language::Korean, "btn_back", "< 뒤로");
    AddTranslation(Language::Korean, "btn_install", "설치 시작");
    AddTranslation(Language::Korean, "btn_uninstall", "제거");
    AddTranslation(Language::Korean, "btn_reinstall", "재설치");
    AddTranslation(Language::Korean, "btn_cancel", "취소");
    AddTranslation(Language::Korean, "btn_finish", "완료");
    AddTranslation(Language::Korean, "btn_browse", "찾아보기...");
    AddTranslation(Language::Korean, "btn_elevate", "Sudo / Polkit으로 관리자 권한 획득");
    AddTranslation(Language::Korean, "lbl_app_name", "응용 프로그램 이름:");
    AddTranslation(Language::Korean, "lbl_app_id", "응용 프로그램 ID (예: org.app.name):");
    AddTranslation(Language::Korean, "lbl_version", "버전:");
    AddTranslation(Language::Korean, "lbl_author", "작성자 / 공급업체:");
    AddTranslation(Language::Korean, "lbl_description", "설명:");
    AddTranslation(Language::Korean, "lbl_category", "카테고리:");
    AddTranslation(Language::Korean, "lbl_license", "라이선스:");
    AddTranslation(Language::Korean, "lbl_install_prefix", "기본 설치 경로:");
    AddTranslation(Language::Korean, "lbl_status", "상태:");
    AddTranslation(Language::Korean, "lbl_progress", "진행률:");
    AddTranslation(Language::Korean, "lbl_source_path", "원본 경로");
    AddTranslation(Language::Korean, "lbl_target_path", "대상 하위 경로");
    AddTranslation(Language::Korean, "lbl_permissions", "권한");
    AddTranslation(Language::Korean, "lbl_dep_name", "종속성 이름:");
    AddTranslation(Language::Korean, "lbl_dep_package", "Apt 패키지 이름:");
    AddTranslation(Language::Korean, "lbl_dep_check_cmd", "확인 명령:");
    AddTranslation(Language::Korean, "lbl_dep_required", "설치 시 필수 항목");
    AddTranslation(Language::Korean, "lbl_pre_install_js", "설치 전 스크립트 (Pre-Install):");
    AddTranslation(Language::Korean, "lbl_post_install_js", "설치 후 스크립트 (예: ~/.config 설정, navi):");
    AddTranslation(Language::Korean, "lbl_pre_uninstall_js", "제거 전 스크립트:");
    AddTranslation(Language::Korean, "lbl_post_uninstall_js", "제거 후 스크립트:");
    AddTranslation(Language::Korean, "lbl_desktop_name", "데스크톱 항목 이름:");
    AddTranslation(Language::Korean, "lbl_desktop_exec", "실행 명령:");
    AddTranslation(Language::Korean, "lbl_desktop_icon", "아이콘:");
    AddTranslation(Language::Korean, "lbl_desktop_categories", "카테고리:");
    AddTranslation(Language::Korean, "lbl_desktop_terminal", "터미널에서 실행");
    AddTranslation(Language::Korean, "lbl_select_output", "출력 설치 프로그램 경로 (.run):");
    AddTranslation(Language::Korean, "lbl_packing", "설치 프로그램을 패키징하는 중...");
    AddTranslation(Language::Korean, "lbl_ready", "준비됨.");
    AddTranslation(Language::Korean, "lbl_installed", "성공적으로 설치되었습니다!");
    AddTranslation(Language::Korean, "lbl_installing", "파일을 설치하는 중...");
    AddTranslation(Language::Korean, "lbl_uninstalling", "구성 요소를 제거하는 중...");
    AddTranslation(Language::Korean, "lbl_welcome", "설치 마법사에 오신 것을 환영합니다.");
    AddTranslation(Language::Korean, "lbl_destination", "설치 대상 폴더 선택:");
    AddTranslation(Language::Korean, "lbl_space_required", "필요한 디스크 공간:");
    AddTranslation(Language::Korean, "lbl_elevation_needed", "이 경로에는 관리자 권한이 필요합니다.");
    AddTranslation(Language::Korean, "lbl_elevation_prompt", "계속하려면 Polkit을 통해 인증하십시오.");
    AddTranslation(Language::Korean, "lbl_deps_checking", "시스템 종속성을 확인하는 중...");
    AddTranslation(Language::Korean, "lbl_deps_ok", "모든 필수 종속성이 충족되었습니다.");
    AddTranslation(Language::Korean, "lbl_deps_missing", "누락된 종속성이 있습니다:");
    AddTranslation(Language::Korean, "lbl_install_complete", "설치가 성공적으로 완료되었습니다.");
    AddTranslation(Language::Korean, "lbl_uninstall_complete", "제거가 성공적으로 완료되었습니다.");
    AddTranslation(Language::Korean, "lbl_launch_app", "응용 프로그램 실행");
    AddTranslation(Language::Korean, "lbl_language", "언어 (Language)");
    AddTranslation(Language::Korean, "lbl_help", "도움말");
    AddTranslation(Language::Korean, "lbl_about", "InstallZone 정보");
    AddTranslation(Language::Korean, "lbl_about_desc", "InstallZone - JPL 표준을 따르는 모듈식 스크립트 지원 설치 시스템.\n그래픽 백엔드: OpenGL2 + Dear ImGui.\n스크립팅: 내장 JavaScript 엔진.\n대상 플랫폼: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::Korean, "lbl_sample_navi", "Navi 설정 코드 조각 삽입");
    AddTranslation(Language::Korean, "lbl_sample_mkdir", "~/.config 폴더 생성 코드 조각 삽입");

    // Russian (Русский)
    AddTranslation(Language::Russian, "app_title_studio", "InstallZone Studio - Создание установщиков");
    AddTranslation(Language::Russian, "app_title_runner", "InstallZone - Установщик приложений");
    AddTranslation(Language::Russian, "tab_general", "Общая информация");
    AddTranslation(Language::Russian, "tab_files", "Файлы и ресурсы");
    AddTranslation(Language::Russian, "tab_dependencies", "Зависимости");
    AddTranslation(Language::Russian, "tab_scripts", "Скрипты JavaScript");
    AddTranslation(Language::Russian, "tab_desktop", "Интеграция с рабочим столом");
    AddTranslation(Language::Russian, "tab_pack", "Сборка установщика");
    AddTranslation(Language::Russian, "btn_new", "Новый проект");
    AddTranslation(Language::Russian, "btn_open", "Открыть проект");
    AddTranslation(Language::Russian, "btn_save", "Сохранить проект");
    AddTranslation(Language::Russian, "btn_save_as", "Сохранить как...");
    AddTranslation(Language::Russian, "btn_exit", "Выход");
    AddTranslation(Language::Russian, "btn_pack_installer", "Собрать автономный установщик");
    AddTranslation(Language::Russian, "btn_add_file", "Добавить файл");
    AddTranslation(Language::Russian, "btn_add_directory", "Добавить каталог");
    AddTranslation(Language::Russian, "btn_remove", "Удалить выбранное");
    AddTranslation(Language::Russian, "btn_clear", "Очистить все");
    AddTranslation(Language::Russian, "btn_add_dep", "Добавить зависимость");
    AddTranslation(Language::Russian, "btn_test_dep", "Проверить зависимость");
    AddTranslation(Language::Russian, "btn_test_script", "Тестовый запуск скрипта (Песочница)");
    AddTranslation(Language::Russian, "btn_next", "Далее >");
    AddTranslation(Language::Russian, "btn_back", "< Назад");
    AddTranslation(Language::Russian, "btn_install", "Установить");
    AddTranslation(Language::Russian, "btn_uninstall", "Удалить");
    AddTranslation(Language::Russian, "btn_reinstall", "Переустановить");
    AddTranslation(Language::Russian, "btn_cancel", "Отмена");
    AddTranslation(Language::Russian, "btn_finish", "Завершить");
    AddTranslation(Language::Russian, "btn_browse", "Обзор...");
    AddTranslation(Language::Russian, "btn_elevate", "Повысить привилегии (Sudo / Polkit)");
    AddTranslation(Language::Russian, "lbl_app_name", "Название приложения:");
    AddTranslation(Language::Russian, "lbl_app_id", "Идентификатор (напр. org.app.name):");
    AddTranslation(Language::Russian, "lbl_version", "Версия:");
    AddTranslation(Language::Russian, "lbl_author", "Автор / Организация:");
    AddTranslation(Language::Russian, "lbl_description", "Описание:");
    AddTranslation(Language::Russian, "lbl_category", "Категория:");
    AddTranslation(Language::Russian, "lbl_license", "Лицензия:");
    AddTranslation(Language::Russian, "lbl_install_prefix", "Каталог установки по умолчанию:");
    AddTranslation(Language::Russian, "lbl_status", "Состояние:");
    AddTranslation(Language::Russian, "lbl_progress", "Прогресс:");
    AddTranslation(Language::Russian, "lbl_source_path", "Исходный путь");
    AddTranslation(Language::Russian, "lbl_target_path", "Целевой подпуть");
    AddTranslation(Language::Russian, "lbl_permissions", "Права доступа");
    AddTranslation(Language::Russian, "lbl_dep_name", "Название зависимости:");
    AddTranslation(Language::Russian, "lbl_dep_package", "Пакет Apt:");
    AddTranslation(Language::Russian, "lbl_dep_check_cmd", "Команда проверки:");
    AddTranslation(Language::Russian, "lbl_dep_required", "Обязательно для установки");
    AddTranslation(Language::Russian, "lbl_pre_install_js", "Скрипт перед установкой (Pre-Install):");
    AddTranslation(Language::Russian, "lbl_post_install_js", "Скрипт после установки (настройка ~/.config, navi):");
    AddTranslation(Language::Russian, "lbl_pre_uninstall_js", "Скрипт перед удалением:");
    AddTranslation(Language::Russian, "lbl_post_uninstall_js", "Скрипт после удаления:");
    AddTranslation(Language::Russian, "lbl_desktop_name", "Имя в меню рабочего стола:");
    AddTranslation(Language::Russian, "lbl_desktop_exec", "Команда запуска:");
    AddTranslation(Language::Russian, "lbl_desktop_icon", "Значок:");
    AddTranslation(Language::Russian, "lbl_desktop_categories", "Категории:");
    AddTranslation(Language::Russian, "lbl_desktop_terminal", "Запускать в терминале");
    AddTranslation(Language::Russian, "lbl_select_output", "Выходной файл установщика (.run):");
    AddTranslation(Language::Russian, "lbl_packing", "Сборка установщика...");
    AddTranslation(Language::Russian, "lbl_ready", "Готово.");
    AddTranslation(Language::Russian, "lbl_installed", "Успешно установлено!");
    AddTranslation(Language::Russian, "lbl_installing", "Установка компонентов...");
    AddTranslation(Language::Russian, "lbl_uninstalling", "Удаление компонентов...");
    AddTranslation(Language::Russian, "lbl_welcome", "Добро пожаловать в мастер установки.");
    AddTranslation(Language::Russian, "lbl_destination", "Выберите каталог для установки:");
    AddTranslation(Language::Russian, "lbl_space_required", "Требуемое место:");
    AddTranslation(Language::Russian, "lbl_elevation_needed", "Для этого каталога требуются права администратора.");
    AddTranslation(Language::Russian, "lbl_elevation_prompt", "Пожалуйста, авторизуйтесь через Polkit.");
    AddTranslation(Language::Russian, "lbl_deps_checking", "Проверка системных зависимостей...");
    AddTranslation(Language::Russian, "lbl_deps_ok", "Все обязательные зависимости удовлетворены.");
    AddTranslation(Language::Russian, "lbl_deps_missing", "Некоторые зависимости отсутствуют:");
    AddTranslation(Language::Russian, "lbl_install_complete", "Установка успешно завершена.");
    AddTranslation(Language::Russian, "lbl_uninstall_complete", "Удаление успешно завершено.");
    AddTranslation(Language::Russian, "lbl_launch_app", "Запустить приложение");
    AddTranslation(Language::Russian, "lbl_language", "Язык (Language)");
    AddTranslation(Language::Russian, "lbl_help", "Справка");
    AddTranslation(Language::Russian, "lbl_about", "О программе InstallZone");
    AddTranslation(Language::Russian, "lbl_about_desc", "InstallZone - Модульная система установщиков по стандартам JPL.\nГрафика: OpenGL2 + Dear ImGui.\nСкрипты: Встроенный движок JavaScript.\nПлатформа: Linux / Kubuntu 26.04 LTS (KDE Plasma).");
    AddTranslation(Language::Russian, "lbl_sample_navi", "Вставить пример настройки Navi");
    AddTranslation(Language::Russian, "lbl_sample_mkdir", "Вставить пример создания ~/.config");
}

void I18n::SetLanguage(Language lang) {
    if (lang >= Language::English && lang < Language::Count) {
        s_current_lang = lang;
    }
}

Language I18n::GetCurrentLanguage() {
    return s_current_lang;
}

const char* I18n::GetLanguageCode(Language lang) {
    int idx = static_cast<int>(lang);
    if (idx >= 0 && idx < static_cast<int>(s_languages.size())) {
        return s_languages[idx].code;
    }
    return "en";
}

const char* I18n::GetLanguageNativeName(Language lang) {
    int idx = static_cast<int>(lang);
    if (idx >= 0 && idx < static_cast<int>(s_languages.size())) {
        return s_languages[idx].native_name;
    }
    return "English";
}

const std::vector<LanguageInfo>& I18n::GetSupportedLanguages() {
    return s_languages;
}

const char* I18n::Tr(const char* key) {
    if (!key) return "";
    int lang_idx = static_cast<int>(s_current_lang);
    auto it = s_tables[lang_idx].find(key);
    if (it != s_tables[lang_idx].end()) {
        return it->second.c_str();
    }
    // Fallback to English
    auto it_en = s_tables[static_cast<int>(Language::English)].find(key);
    if (it_en != s_tables[static_cast<int>(Language::English)].end()) {
        return it_en->second.c_str();
    }
    return key;
}

void I18n::LoadFonts(float fontSize) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    // Candidates for base font (Cyrillic + Latin)
    const char* base_font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf"
    };

    const char* selected_base = nullptr;
    for (const char* p : base_font_paths) {
        if (access(p, R_OK) == 0) {
            selected_base = p;
            break;
        }
    }

    if (selected_base) {
        ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 2;
        config.PixelSnapH = true;

        // Build Cyrillic + Latin glyph ranges
        static const ImWchar base_ranges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0100, 0x017F, // Latin Extended-A
            0x0400, 0x052F, // Cyrillic + Cyrillic Supplementary (covers Ukrainian, Russian)
            0x2DE0, 0x2DFF, // Cyrillic Extended-A
            0xA640, 0xA69F, // Cyrillic Extended-B
            0x2000, 0x206F, // General Punctuation
            0
        };

        io.Fonts->AddFontFromFileTTF(selected_base, fontSize, &config, base_ranges);

        // Merge CJK font if available for Japanese, Chinese, Korean
        const char* cjk_font_paths[] = {
            "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
            "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc",
            "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf"
        };

        const char* selected_cjk = nullptr;
        for (const char* p : cjk_font_paths) {
            if (access(p, R_OK) == 0) {
                selected_cjk = p;
                break;
            }
        }

        if (selected_cjk) {
            ImFontConfig cjk_config;
            cjk_config.MergeMode = true;
            cjk_config.OversampleH = 1;
            cjk_config.OversampleV = 1;
            cjk_config.PixelSnapH = true;

            // Load CJK glyph ranges
            static const ImWchar cjk_ranges[] = {
                0x3000, 0x303F, // CJK Symbols and Punctuation
                0x3040, 0x309F, // Hiragana
                0x30A0, 0x30FF, // Katakana
                0x31F0, 0x31FF, // Katakana Phonetic Extensions
                0x4E00, 0x9FAF, // CJK Unified Ideographs (Common Japanese & Chinese)
                0xAC00, 0xD7AF, // Hangul Syllables (Korean)
                0x1100, 0x11FF, // Hangul Jamo
                0xFF00, 0xFFEF, // Halfwidth and Fullwidth Forms
                0
            };

            io.Fonts->AddFontFromFileTTF(selected_cjk, fontSize, &cjk_config, cjk_ranges);
        }
    } else {
        // Fallback default ImGui font
        io.Fonts->AddFontDefault();
    }

    io.Fonts->Build();
}

} // namespace iz

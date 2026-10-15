#pragma once

#include <string>
#include <vector>

struct ImFont;

namespace iz {

enum class Language : int {
    English = 0,
    Ukrainian,
    Japanese,
    Chinese,
    Spanish,
    French,
    German,
    Portuguese,
    Italian,
    Polish,
    Korean,
    Russian,
    Count
};

struct LanguageInfo {
    Language lang;
    const char* code;
    const char* english_name;
    const char* native_name;
};

class I18n {
public:
    static void Initialize();
    static void SetLanguage(Language lang);
    static Language GetCurrentLanguage();
    static const char* GetLanguageCode(Language lang);
    static const char* GetLanguageNativeName(Language lang);
    static const std::vector<LanguageInfo>& GetSupportedLanguages();

    // Translate string key
    static const char* Tr(const char* key);

    // Setup ImGui fonts for multilingual UTF-8 support
    static void LoadFonts(float fontSize = 16.0f);

private:
    static Language s_current_lang;
};

// Convenience macro for localization
#define _(key) ::iz::I18n::Tr(key)

} // namespace iz

#pragma once

#include "jpl_defs.h"
#include <string>

namespace iz {

class AudioManager {
public:
    static bool Initialize();
    static void Shutdown();

    static bool LoadMusic(const std::string& wav_filepath);
    static void PlayMusic(bool loop = true);
    static void StopMusic();

    static void SetMuted(bool muted);
    static bool IsMuted();

    static void SetVolume(float volume); // 0.0f to 1.0f (default 0.5f)
    static float GetVolume();

    static bool IsPlaying();

    // Auto-search and start default chiptune
    static bool StartDefaultChiptune();
};

} // namespace iz

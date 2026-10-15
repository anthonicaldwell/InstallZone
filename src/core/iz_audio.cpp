#include "iz_audio.h"
#include <SDL2/SDL.h>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <vector>

namespace iz {

struct AudioState {
    SDL_AudioDeviceID device_id{0};
    SDL_AudioSpec spec;
    Uint8* wav_buffer{nullptr};
    Uint32 wav_length{0};
    Uint32 current_pos{0};
    float volume{0.50f}; // 50% volume by default
    bool muted{false};
    bool playing{false};
    bool loop{true};
    std::mutex mutex;
};

static AudioState s_audio;

static void SDLCALL AudioCallback(void* userdata, Uint8* stream, int len) {
    AudioState* state = static_cast<AudioState*>(userdata);
    std::lock_guard<std::mutex> lock(state->mutex);

    std::memset(stream, 0, len);
    if (!state->playing || state->muted || state->wav_length == 0 || state->volume <= 0.0f) {
        return;
    }

    int remaining = len;
    Uint8* dest = stream;

    while (remaining > 0) {
        Uint32 avail = state->wav_length - state->current_pos;
        int chunk = (remaining < static_cast<int>(avail)) ? remaining : static_cast<int>(avail);

        int mix_vol = static_cast<int>(state->volume * SDL_MIX_MAXVOLUME);
        SDL_MixAudioFormat(dest, state->wav_buffer + state->current_pos, state->spec.format, chunk, mix_vol);

        dest += chunk;
        remaining -= chunk;
        state->current_pos += chunk;

        if (state->current_pos >= state->wav_length) {
            if (state->loop) {
                state->current_pos = 0;
            } else {
                state->playing = false;
                break;
            }
        }
    }
}

bool AudioManager::Initialize() {
    if (s_audio.device_id != 0) return true;

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            return false;
        }
    }
    return true;
}

void AudioManager::Shutdown() {
    StopMusic();
    std::lock_guard<std::mutex> lock(s_audio.mutex);
    if (s_audio.device_id != 0) {
        SDL_CloseAudioDevice(s_audio.device_id);
        s_audio.device_id = 0;
    }
    if (s_audio.wav_buffer != nullptr) {
        SDL_FreeWAV(s_audio.wav_buffer);
        s_audio.wav_buffer = nullptr;
        s_audio.wav_length = 0;
    }
    s_audio.current_pos = 0;
}

bool AudioManager::LoadMusic(const std::string& wav_filepath) {
    if (!Initialize()) return false;
    if (access(wav_filepath.c_str(), R_OK) != 0) return false;

    std::lock_guard<std::mutex> lock(s_audio.mutex);

    if (s_audio.device_id != 0) {
        SDL_CloseAudioDevice(s_audio.device_id);
        s_audio.device_id = 0;
    }
    if (s_audio.wav_buffer != nullptr) {
        SDL_FreeWAV(s_audio.wav_buffer);
        s_audio.wav_buffer = nullptr;
    }

    SDL_AudioSpec desired;
    if (SDL_LoadWAV(wav_filepath.c_str(), &desired, &s_audio.wav_buffer, &s_audio.wav_length) == nullptr) {
        return false;
    }

    desired.callback = AudioCallback;
    desired.userdata = &s_audio;

    s_audio.device_id = SDL_OpenAudioDevice(nullptr, 0, &desired, &s_audio.spec, 0);
    if (s_audio.device_id == 0) {
        SDL_FreeWAV(s_audio.wav_buffer);
        s_audio.wav_buffer = nullptr;
        s_audio.wav_length = 0;
        return false;
    }

    s_audio.current_pos = 0;
    return true;
}

void AudioManager::PlayMusic(bool loop) {
    std::lock_guard<std::mutex> lock(s_audio.mutex);
    if (s_audio.device_id == 0 || s_audio.wav_buffer == nullptr) return;

    s_audio.loop = loop;
    s_audio.playing = true;
    SDL_PauseAudioDevice(s_audio.device_id, 0);
}

void AudioManager::StopMusic() {
    std::lock_guard<std::mutex> lock(s_audio.mutex);
    if (s_audio.device_id != 0) {
        SDL_PauseAudioDevice(s_audio.device_id, 1);
    }
    s_audio.playing = false;
    s_audio.current_pos = 0;
}

void AudioManager::SetMuted(bool muted) {
    std::lock_guard<std::mutex> lock(s_audio.mutex);
    s_audio.muted = muted;
    if (s_audio.device_id != 0) {
        SDL_PauseAudioDevice(s_audio.device_id, muted ? 1 : 0);
    }
}

bool AudioManager::IsMuted() {
    std::lock_guard<std::mutex> lock(s_audio.mutex);
    return s_audio.muted;
}

void AudioManager::SetVolume(float volume) {
    std::lock_guard<std::mutex> lock(s_audio.mutex);
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    s_audio.volume = volume;
}

float AudioManager::GetVolume() {
    std::lock_guard<std::mutex> lock(s_audio.mutex);
    return s_audio.volume;
}

bool AudioManager::IsPlaying() {
    std::lock_guard<std::mutex> lock(s_audio.mutex);
    return s_audio.playing && !s_audio.muted;
}

bool AudioManager::StartDefaultChiptune(const std::string& override_path) {
    if (!override_path.empty() && access(override_path.c_str(), R_OK) == 0) {
        if (LoadMusic(override_path)) {
            PlayMusic(true);
            return true;
        }
    }

    const char* home = std::getenv("HOME");
    std::vector<std::string> paths = {
        (home ? std::string(home) + "/.config/InstallZone/assets/chiptune.wav" : ""),
        "assets/chiptune.wav",
        "../assets/chiptune.wav",
        "/usr/share/installzone/assets/chiptune.wav",
        "/usr/local/share/installzone/assets/chiptune.wav"
    };

    for (const auto& p : paths) {
        if (!p.empty() && access(p.c_str(), R_OK) == 0) {
            if (LoadMusic(p)) {
                PlayMusic(true);
                return true;
            }
        }
    }

    return false;
}

} // namespace iz

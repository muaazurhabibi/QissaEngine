#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "../common.hpp"

#define MAX_AUDIO_ALLOWED 20

class AudioEngine
{
    private:
        SDL_AudioDeviceID audio_device;
        AudioData *audios = (AudioData *)malloc(sizeof(AudioData) * MAX_AUDIO_ALLOWED);
        int n_audios = 0;

    public:
        int last_audio_played_id = -1;

        AudioEngine();
        void init_device();
        void load_wav(int id, char *file_loc);
        void play_wav(int audio_idx, float vol, float fade_speed);
        void play_wav_once(int audio_idx, float vol, float fade_speed);
        void stop_wav(int audio_idx, float fade_speed);
        void update_audios();
};

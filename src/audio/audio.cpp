#include "audio.hpp"

AudioEngine::AudioEngine()
{

}

void AudioEngine::init_device()
{
    audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!audio_device)
    {
        printf("Failed to open audio device, %s\n", SDL_GetError());
    }
}

void AudioEngine::load_wav(int id, char *file_loc)
{
    if (!SDL_LoadWAV(file_loc, &audios[n_audios].spec, &audios[n_audios].wav_data, &audios[n_audios].wav_len))
    {
        printf("Error loading wav file\n");
    }

    audios[n_audios].stream = SDL_CreateAudioStream(&audios[n_audios].spec, NULL);
    audios[n_audios].id = id;

    if (!SDL_BindAudioStream(audio_device, audios[n_audios].stream))
    {
        printf("Failed to bind stream to device, %s\n", SDL_GetError());
    }
    n_audios++;
}

void AudioEngine::play_wav(int audio_id, float vol, float fade_speed)
{
    AudioData *a = NULL;
    for (size_t i = 0; i < n_audios; i++)
    {
        if (audios[i].id == audio_id)
        {
            a = &audios[i];
            break;
        }
    }

    if (!SDL_PutAudioStreamData(a->stream, a->wav_data, a->wav_len))
    {
        printf("Failed to queue audio data, %s\n", SDL_GetError());
    }

    a->is_playing = true;
    a->loop = false;

    a->vol = 0.1, a->max_vol = vol, a->fade_dir = 1, a->fade_speed = fade_speed;

    SDL_SetAudioStreamGain(a->stream, a->vol);
    SDL_ResumeAudioStreamDevice(a->stream);
    last_audio_played_id = audio_id;
}

void AudioEngine::play_wav_once(int audio_id, float vol, float fade_speed)
{
    if (last_audio_played_id != audio_id)
    {
        play_wav(audio_id, vol, fade_speed);
    }
}

void AudioEngine::stop_wav(int audio_id, float fade_speed)
{
    AudioData *a = NULL;
    for (size_t i = 0; i < n_audios; i++)
    {
        if (audios[i].id ==  audio_id)
        {
            a = &audios[i];
            break;
        }
    }

    //SDL_ClearAudioStream(a->stream);
    //a->is_playing = false;

    a->fade_speed = fade_speed, a->fade_dir = -1;
}

void AudioEngine::update_audios()
{
    for (size_t i = 0; i < n_audios; i++)
    {
        if (audios[i].is_playing)
        {
            if (!SDL_GetAudioStreamQueued(audios[i].stream))
            {
                audios[i].is_playing = false;
            } else {
                audios[i].vol += audios[i].fade_speed * audios[i].fade_dir;
                SDL_SetAudioStreamGain(audios[i].stream, audios[i].vol);

                if (audios[i].vol < 0.0f)
                {
                    audios[i].vol = 0.0;
                    audios[i].fade_dir = 0;
                    SDL_ClearAudioStream(audios[i].stream);
                    audios[i].is_playing = false;
                } else
                if (audios[i].vol > 1.0f)
                {
                    audios[i].vol = 1.0;
                    audios[i].fade_dir = 0;
                }
            }
        }
    }
}

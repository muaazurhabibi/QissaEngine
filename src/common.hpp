#pragma once

#include <SDL3/SDL.h>

#define MAX_SPRITE_AMOUNT 500

#define TEXT_BOX_CHARS 1024

#define FLOATARRAY_POSX 0
#define FLOATARRAY_POSY 1
#define FLOATARRAY_SPEED 5
#define FLOATARRAY_FADE 4
#define FLOATARRAY_SCALEX 2
#define FLOATARRAY_SCALEY 3
#define FLOATARRAY_MOVETOX 6
#define FLOATARRAY_VOL 7

enum Texture_Type {TEXTURE_TOPMOST, TEXTURE_TYPE_TEXT, TEXTURE_TYPE_IMAGE, TEXTURE_EMPTY};
enum Sprite_Type {CHARACTER_SPRITE, ITEM_SPRITE, BACKGROUND_SPRITE, EMPTY_SPRITE};
// for some reason handler was returning the first value by default if returning an empty token, idk why
enum Renderer_Command {DONT_USE_THIS, WINDOW_WIDTH_ADJUSTMENT, WINDOW_HEIGHT_ADJUSTMENT, SHOW_STARTMENU,
                        MENU_BUTTON_ADJUSTMENT, CREATE_SPRITE, CREATE_ANIMATION, PLAY_ANIMATION, STOP_ANIMATION,
                        LOAD_AUDIO, PLAY_AUDIO, SET_BUTTON_PARAM, MOVE_SPRITE, WAITFOR_DELAY, CROSSFADE_SPRITE, STOP_AUDIO,
                        ADD_SPRITE_TO_ARRAY, ADD_SPRITE_TO_SCENE, SHOW_TEXT, CLEAR_TEXT, WAITFOR_USERINPUT,
                        WAITFOR_BUTTONCALLBACK, CONTINUE, SHOW_CHOICES, REMOVE_SPRITE_FROM_SCENE, END_OF_GAME};

struct Token {
    char command[32];
    char opcode[32];
    char additional_params[512];
};

struct Sprite {
    char name[64];
    SDL_Texture *texture;
    Sprite_Type type;
};

struct Texture {
    char identifier[64];
    SDL_Texture *texture;
    float alpha, fade_speed;
    int fade_dir; // 0 for none, 1 for in, -1 for out
    float speed, move_to_x; // speed to move, and the x pos to move to
    Texture_Type type;
    Sprite_Type sprite_type;
    SDL_FRect transformation; // x and y are positions, w and h are scales
    int sprite_idx; // index of texture in sprites array
};

struct Handler_Return {
    Renderer_Command command;
    char string_data[sizeof(char) * TEXT_BOX_CHARS];
    int integer_data;
    float float_array[8];
    Sprite sprite_data;
    Texture texture_data;
    Token token_data;
};

struct AudioData {
    char *name;
    int id;
    SDL_AudioStream *stream;
    SDL_AudioSpec spec;
    Uint8 *wav_data;
    Uint32 wav_len;
    bool is_playing;
    bool loop;
    float vol, max_vol, fade_speed;
    int fade_dir;
};

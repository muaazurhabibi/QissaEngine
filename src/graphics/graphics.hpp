#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <cstdlib>
#include "../audio/audio.hpp"
#include "../common.hpp"
#include "../handler.hpp"

#define TRUE_ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define MAX_SCENE_OBJECTS 20
#define MAX_WINDOWS 10

#define SETTINGS_WINDOW_IDX 0
#define CONTINUE_WINDOW_IDX 1

#define TEXT_BOX_PADDING 20

typedef void (*Button_Callback)();
typedef void (*Slider_Callback)(float value);
typedef struct {
    SDL_Texture *speaker;
    SDL_Texture *text;
} text_texture_ret;

/**/

const int TEXT_BOX_POS_X = 150;
const int TEXT_BOX_POS_Y = 540;
const float CHOICES_BUTTON_START = 200;
const float CHOICES_BUTTON_MARGIN = 40;

class Handler;

class Label {
    private:
        SDL_Texture *text_texture;
        SDL_FRect rect;
    
    public:
        Label() {}
        Label(SDL_Texture *text_t, SDL_FRect pos_size) {
            text_texture = text_t;
            rect.x = pos_size.x;
            rect.y = pos_size.y;
        }

        SDL_Texture *get_texture() { return text_texture; }
        SDL_FRect get_rect() { return rect; }
        void update_rect(SDL_FRect updated) { rect = updated; }
};

class Button
{
    private:
        bool used_for_choice;
        char text[64];
        SDL_Texture *text_texture = NULL;
        SDL_Texture *button_texture = NULL;
        SDL_FRect rect;
        SDL_FRect text_rect;
        float text_w, text_h;
        int choice_number;
        Button_Callback callback = NULL;

    public:
        bool hovered_last_frame = false;

        Button() {}
        Button(char *b_text, int choice, SDL_FRect pos_size, bool used_for_choices)
        {
            strncpy(text, b_text, strlen(b_text)); text[strlen(b_text)] = '\0';
            rect = pos_size;
            text_rect.y = rect.y;
            text_rect.x = rect.x;
            choice_number = choice;
            used_for_choice = used_for_choices;
        }

        bool is_button_choice() { return used_for_choice; }

        void set_callback(Button_Callback cb) { callback = cb; }
        void call_callback() { if (callback) { callback(); } else { printf("no callback assigned\n"); } }

        char *get_text() { return text; }
        SDL_Texture *get_button_texture() { return button_texture; }
        SDL_FRect get_rect() { return rect; }
        int get_choice_number() { return choice_number; }

        void set_button_texture(SDL_Texture *tex) { button_texture = tex; }
        void set_text_texture(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color)
        {
            SDL_Surface *surface = TTF_RenderText_Blended(font, text, 0, color);
            text_texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_DestroySurface(surface);
        }

        void render_button(SDL_Renderer *renderer)
        {
            SDL_RenderTexture(renderer, button_texture, NULL, &rect);
            SDL_SetTextureBlendMode(text_texture, SDL_BLENDMODE_BLEND);
            SDL_GetTextureSize(text_texture, &text_rect.w, &text_rect.h);
            //text_rect.x = (rect.w - text_rect.w)/2; // horizontal centering, optional
            text_rect.y = rect.y + (rect.h - text_rect.h)/2; // vertical centering, necessary
            SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
        }
        bool check_button_hovered(float mouse_x, float mouse_y)
        {
            if (mouse_x >= rect.x && mouse_x <= rect.x+rect.w && mouse_y >= rect.y && mouse_y <= rect.y+rect.h)
            {
                // we can switch texture and/or play a sound on hover
                // play sound is implement in main loop since we also need to check if it was hovered last frame to avoid replaying sound every frame while hovering
                return true;
            }
            return false;
        }
};


// a.i generated slider class
class Slider
{
private:
    char label[64];
    SDL_Texture *label_texture = NULL;
    SDL_Texture *background_texture = NULL;
    SDL_Texture *handle_texture = NULL;

    SDL_FRect background_rect;     // Full slider background
    SDL_FRect handle_rect;         // Draggable handle
    SDL_FRect label_rect;          // Label text position

    float min_value;
    float max_value;
    float current_value;

    bool is_dragging;
    Slider_Callback callback;

    // Visual properties
    float handle_width;
    float handle_height;

public:
    //Slider() : min_value(0.0f), max_value(100.0f), current_value(50.0f), is_dragging(false), callback(NULL), handle_width(20.0f), handle_height(30.0f) {}
    Slider(const char* lbl, SDL_FRect bg_rect, float min_val, float max_val, float default_val)
    {
        strncpy(label, lbl, 63);
        label[63] = '\0';
        background_rect = bg_rect;
        min_value = min_val;
        max_value = max_val;
        current_value = default_val;
        is_dragging = false;
        callback = NULL;

        // Initialize handle dimensions (smaller than background)
        handle_width = bg_rect.h * 0.8f;
        handle_height = bg_rect.h * 0.8f;

        // Position handle based on current value
        update_handle_position();

        // Setup label position (above the slider)
        label_rect.x = bg_rect.x;
        label_rect.y = bg_rect.y - 25;
        label_rect.w = 0;  // Will be set when texture is created
        label_rect.h = 0;
    }

    // Getters
    float get_value() { return current_value; }
    int get_value_int() { return (int)current_value; }
    SDL_FRect get_rect() { return background_rect; }
    bool is_dragging_slider() { return is_dragging; }

    // Setters
    void set_callback(Slider_Callback cb) { callback = cb; }
    void set_background_texture(SDL_Texture *tex) { background_texture = tex; }
    void set_handle_texture(SDL_Texture *tex) { handle_texture = tex; }

    void set_label_texture(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color)
    {
        SDL_Surface *surface = TTF_RenderText_Blended(font, label, 0, color);
        label_texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (label_texture) {
            SDL_GetTextureSize(label_texture, &label_rect.w, &label_rect.h);
        }
        SDL_DestroySurface(surface);
    }

    void set_value(float value)
    {
        // Clamp value to range
        if (value < min_value) value = min_value;
        if (value > max_value) value = max_value;

        if (current_value != value) {
            current_value = value;
            update_handle_position();

            // Call callback if assigned
            if (callback) {
                callback(current_value);
            }
        }
    }

    void set_value_normalized(float normalized)  // 0.0 to 1.0
    {
        float value = min_value + (normalized * (max_value - min_value));
        set_value(value);
    }

private:
    void update_handle_position()
    {
        // Calculate handle X position based on current value
        float t = (current_value - min_value) / (max_value - min_value);
        float handle_x = background_rect.x + (t * (background_rect.w - handle_width));

        handle_rect.x = handle_x;
        handle_rect.y = background_rect.y + (background_rect.h - handle_height) / 2;
        handle_rect.w = handle_width;
        handle_rect.h = handle_height;
    }

    void update_value_from_position(float mouse_x)
    {
        // Calculate value based on handle position
        float min_x = background_rect.x;
        float max_x = background_rect.x + background_rect.w - handle_width;

        // Clamp mouse position to slider bounds
        if (mouse_x < min_x) mouse_x = min_x;
        if (mouse_x > max_x) mouse_x = max_x;

        // Calculate normalized position (0 to 1)
        float t = (mouse_x - min_x) / (max_x - min_x);

        // Convert to actual value
        float new_value = min_value + (t * (max_value - min_value));
        set_value(new_value);
    }

public:
    bool handle_event(SDL_Event &event, float mouse_x, float mouse_y)
    {
        // Mouse button down - start dragging
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            // Check if clicking on handle
            if (mouse_x >= handle_rect.x && mouse_x <= handle_rect.x + handle_rect.w &&
                mouse_y >= handle_rect.y && mouse_y <= handle_rect.y + handle_rect.h)
            {
                printf("starting drag\n");
                is_dragging = true;
                return true;
            }
            // Check if clicking on background bar (jump to position)
            else if (mouse_x >= background_rect.x && mouse_x <= background_rect.x + background_rect.w &&
                    mouse_y >= background_rect.y && mouse_y <= background_rect.y + background_rect.h)
            {
                // Calculate where the handle's CENTER should be
                float handle_center_x = mouse_x;

                // Convert to position (accounting for handle width)
                float new_handle_x = handle_center_x - (handle_width / 2);

                // Clamp to valid range
                float min_x = background_rect.x;
                float max_x = background_rect.x + background_rect.w - handle_width;
                if (new_handle_x < min_x) new_handle_x = min_x;
                if (new_handle_x > max_x) new_handle_x = max_x;

                // Calculate value from position
                float t = (new_handle_x - min_x) / (max_x - min_x);
                float new_value = min_value + (t * (max_value - min_value));

                set_value(new_value);
                is_dragging = true;
                return true;
            }
        }

        // Mouse button up - stop dragging
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT)
        {
            if (is_dragging) {
                is_dragging = false;
                return true;
            }
        }

        // Mouse motion - update value while dragging
        if (event.type == SDL_EVENT_MOUSE_MOTION && is_dragging)
        {
            // Calculate handle position directly from mouse
            float new_handle_x = mouse_x - (handle_width / 2);

            // Clamp to valid range
            float min_x = background_rect.x;
            float max_x = background_rect.x + background_rect.w - handle_width;
            if (new_handle_x < min_x) new_handle_x = min_x;
            if (new_handle_x > max_x) new_handle_x = max_x;

            // Calculate value from position
            float t = (new_handle_x - min_x) / (max_x - min_x);
            float new_value = min_value + (t * (max_value - min_value));

            set_value(new_value);
            return true;
        }

        return false;
    }

    void render(SDL_Renderer *renderer)
    {
        // Draw background bar
        if (background_texture) {
            SDL_RenderTexture(renderer, background_texture, NULL, &background_rect);
        } else {
            // Fallback: draw a colored rectangle
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderFillRect(renderer, &background_rect);
        }

        // Draw filled portion (optional - shows progress)
        SDL_FRect filled_rect = background_rect;
        filled_rect.w = (handle_rect.x + handle_width/2) - background_rect.x;
        if (filled_rect.w > 0) {
            SDL_SetRenderDrawColor(renderer, 150, 150, 255, 180);
            SDL_RenderFillRect(renderer, &filled_rect);
        }

        // Draw handle
        if (handle_texture) {
            SDL_RenderTexture(renderer, handle_texture, NULL, &handle_rect);
        } else {
            // Fallback: draw a colored rectangle
            if (is_dragging) {
                SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);  // Red when dragging
            } else {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);  // Gray normally
            }
            SDL_RenderFillRect(renderer, &handle_rect);

            // Draw border
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderRect(renderer, &handle_rect);
        }

        // Draw label
        if (label_texture) {
            SDL_RenderTexture(renderer, label_texture, NULL, &label_rect);
        }

        // Draw value text (optional - you can add this as a texture)
        char value_text[32];
        snprintf(value_text, sizeof(value_text), "%.0f", current_value);
        // You would need to render this as text similarly to the label
    }

    bool check_hovered(float mouse_x, float mouse_y)
    {
        return (mouse_x >= background_rect.x && mouse_x <= background_rect.x + background_rect.w &&
                mouse_y >= background_rect.y && mouse_y <= background_rect.y + background_rect.h);
    }
};

class Window
{
    private:
        SDL_FRect window;
        SDL_Color color;
        SDL_Texture *texture;
        SDL_Texture *title_texture;
        Label *labels = NULL;
        Button *buttons = NULL;
        Slider *sliders = NULL;
        int n_labels, n_buttons, n_sliders;

    public:
        Window() {}
        Window(SDL_Texture *title, SDL_FRect dims, SDL_Color bg_color, SDL_Texture *bg_texture,
            Label *window_labels, int n_window_labels,
            Button *window_buttons, int n_window_buttons,
            Slider *window_sliders, int n_window_sliders)
        {
            window = dims;
            color = bg_color;
            texture = bg_texture;
            title_texture = title;

            if (window_labels != NULL)
            {
                n_labels = n_window_labels;
                labels = (Label *)malloc(sizeof(Label) * n_window_labels);
                for (size_t i = 0; i < n_window_labels; i++)
                {
                    labels[i] = window_labels[i];
                }
            }
            if (window_buttons != NULL)
            {
                n_buttons = n_window_buttons;
                buttons = (Button *)malloc(sizeof(Button) * n_window_buttons);
                for (size_t i = 0; i < n_window_buttons; i++)
                {
                    buttons[i] = window_buttons[i];
                }
            }
        }
        void render_window(SDL_Renderer *renderer)
        {
            if (color.a != 0)
            {
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(renderer, &window);

                float title_w, title_h;
                SDL_GetTextureSize(title_texture, &title_w, &title_h);

                SDL_FRect dst = (SDL_FRect){.x=window.x+10, .y=window.y-title_h, .w=title_w, .h=title_h};
                SDL_RenderTexture(renderer, title_texture, NULL, &dst);
            } else
            {
                SDL_RenderTexture(renderer, texture, NULL, &window);
                SDL_RenderRect(renderer, &window);
            }

            if (labels != NULL)
            {
                render_labels(renderer);
            }
        }

        void render_labels(SDL_Renderer *renderer)
        {
            for (size_t i = 0; i < n_labels; i++)
            {
                Label l = labels[i];
                float w, h;
                SDL_GetTextureSize(l.get_texture(), &w, &h);

                SDL_FRect fixed = SDL_FRect{.x=l.get_rect().x+window.x, .y=l.get_rect().y+window.y, .w=w, .h=h};

                SDL_RenderTexture(renderer, l.get_texture(), NULL, &fixed);
            }
        }

        void render_buttons(SDL_Renderer *renderer)
        {
            for (size_t i = 0; i < n_buttons; i++)
            {
                Button b = buttons[i];
                float w, h;
            }
        }

        SDL_FRect get_dims() { return window; }
};

class Renderer
{
    private:
        static Renderer *inst;
        SDL_Window *window;
        SDL_Renderer *renderer;
        int window_w, window_h;
        bool running;

        Texture *scene_objects;
        int n_scene_objects;

        Sprite *sprites;
        int n_sprites;

        Button *choice_buttons;
        int n_choice_buttons;

        Window *windows;
        int n_windows; int window_to_render=-1;
        bool show_window = false;

        SDL_Texture **current_animation_frames;
        int n_current_animation_frames;
        bool play_animation;
        int current_frame, delay_bw_frames;

        AudioEngine audio_engine;
        int btn_hover_audio_id = 0;
        int btn_click_audio_id = 0;

        SDL_Texture *text_box_texture = NULL;
        SDL_FRect text_box;
        TTF_Font *font = NULL;
        float tb_anim_speed = 19.0; // control how fast text box opens and closes
        float cur_tb_h = 0.0; // used for animating opening and cloosing of text box
        float threshold_tb_text = 10.0; // when height of text box reaches this only then show/hide text
        char full_text[256];
        int last_text_pos = 0;
        int text_scroll_speed = 5;
        SDL_Texture *speaker_texture = NULL;
        SDL_Texture *text_texture = NULL;

        static void start_button_callback()
        {
            if (inst)
            {
                inst->should_pause = false;
                inst->choice_buttons = NULL;
                inst->n_choice_buttons = 0;
            }
        }
        static void continue_button_callback()
        {
            if (inst)
            {
                inst->show_window = true;
                inst->window_to_render = CONTINUE_WINDOW_IDX;
            }
        }
        static void settings_button_callback()
        {
            if (inst)
            {
                inst->show_window = true;
                inst->window_to_render = SETTINGS_WINDOW_IDX;
            }
        }
        static void exit_button_callback()
        {
            if (inst)
            {
                inst->running = false;
            }
        }

    public:
        // some needed public variables
        bool should_pause = false;
        bool skippable_pause = true;

        // helper function (mostly for debugging) (also mostly ai generated)
        void print_sprites()
        {
            printf("\n===== SPRITES (%d) =====\n", n_sprites);
            for (int i = 0; i < n_sprites; i++)
            {
                printf("Sprite[%d]\n", i);
                printf("  name: %s\n", sprites[i].name);
                printf("  type: %d\n", sprites[i].type);
                if (sprites[i].texture)
                    printf("  texture: %p\n", (void*)sprites[i].texture);
                else
                    printf("  texture: NULL\n");
                printf("\n");
            }
        }
        void print_textures()
        {
            printf("\n===== TEXTURES (%d) =====\n", n_scene_objects);
            for (int i = 0; i < n_scene_objects; i++)
            {
                printf("Texture[%d]\n", i);
                printf("  identifier: %s\n", scene_objects[i].identifier);
                printf("  type: %d\n", scene_objects[i].type);
                printf("  sprite_type: %d\n", scene_objects[i].sprite_type);
                printf("  sprite_idx: %d\n", scene_objects[i].sprite_idx);
                if (scene_objects[i].texture)
                    printf("  texture: %p\n", (void*)scene_objects[i].texture);
                else
                    printf("  texture: NULL\n");
                printf("\n");
            }
        }

        // getters
        int get_n_sprites();
        int get_n_scene_objects();
        Sprite *get_sprites();
        SDL_Texture **get_scene_objects();

        Renderer(int width, int height);
        void init_graphics_and_audio();
        void create_main_window(const char *title);
        void main_graphics_loop(int fps, Handler &handler);
        void create_renderer();
        SDL_Texture *create_image(char *file_loc);
        SDL_Texture **create_animation(char *file_loc, int *n_frames, int *delay);

        void add_startmenu_button(Button button);

        void create_sprite(char *name, char *image_filepath, Sprite_Type sprite_type);
        void add_sprite(Sprite sprite);
        int add_sprite_to_scene(Sprite sprite, SDL_FRect pos_and_scale, float fade_speed, float move_speed, float move_to_x, int sprite_idx);
        void remove_sprite_from_scene(char *name);
        void fade_sprite(Texture &t);
        void move_sprite(Texture &t, float finalx);

        void create_text_box(const char *text_box_image, const char *font_file);
        text_texture_ret show_text(char *text, bool for_textbox);
        void show_next_text_part();

        void show_choices(int n_choices, char *choices_csv);

        static uint32_t delay_timer_callback(void *userdata, SDL_TimerID timerid, uint32_t interval)
        {
            printf("timer_callback fired\n");
            inst->should_pause = false;
            return 0;
        }
};

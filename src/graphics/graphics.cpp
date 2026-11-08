#include "graphics.hpp"

/*
    The main graphics renderer, renders almost everything (sprites, backgrounds, buttons) and handles all input events
*/

/*
    - in the function show_text the shadow offset is causing text in Window class's title textures to be slightly clipped,
    for now just add a space before the title
*/

Renderer *Renderer::inst = nullptr;

// Getters
// Returns the current number of sprites stored in the sprites array.
int Renderer::get_n_sprites() { return n_sprites; }

// Returns the current number of objects present in the scene.
int Renderer::get_n_scene_objects() { return n_scene_objects; }

// Returns a pointer to the sprites array.
Sprite *Renderer::get_sprites() { return sprites; }

// Allocates and returns a heap array of SDL_Texture pointers extracted from scene_objects; caller is responsible for freeing.
SDL_Texture **Renderer::get_scene_objects() {
    SDL_Texture **r = (SDL_Texture**)malloc(sizeof(SDL_Texture*) * n_scene_objects);
    for (size_t i = 0; i < n_scene_objects; i++)
    {
        r[i] = scene_objects[i].texture;
    }

    return r;
}

// initialisation related functions
// Constructs the Renderer, allocates scene_objects and sprites buffers, and zeroes out all state; takes window width and height.
Renderer::Renderer(int width, int height)
{
    inst = this;
    window_w=width; window_h=height;

    scene_objects = (Texture*)malloc(sizeof(Texture) * MAX_SCENE_OBJECTS);
    n_scene_objects = 0;

    sprites = (Sprite*)malloc(sizeof(Sprite) * MAX_SPRITE_AMOUNT);
    n_sprites = 0;

    running = true;

    choice_buttons = NULL;
    n_choice_buttons = 0;

    windows = (Window*)malloc(sizeof(Window) * MAX_WINDOWS);
    n_windows = 0;

    play_animation = false;
    current_frame = 0;
}

// Initialises TTF, SDL video/audio subsystems, and the audio engine device; prints errors on failure.
void Renderer::init_graphics_and_audio()
{
    if (TTF_Init() < 0) {
        printf("TTF_Init failed\n");
    }

    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        printf("SDL_Init failed\n");
    }

    audio_engine.init_device();
}

// Creates and stores the main SDL window with the given title at the configured width and height.
void Renderer::create_main_window(const char *title)
{
    SDL_Window *win = SDL_CreateWindow(title, window_w ,window_h, 0);
    if (!win) {
        printf("Error in creating SDL window\n");
    }

    window = win;
}

// Creates and stores the SDL renderer attached to the main window.
void Renderer::create_renderer()
{
    SDL_Renderer *ren = SDL_CreateRenderer(window, NULL);
    if (!ren) {
        printf("Error in creating renderer, %s\n", SDL_GetError());
    }
    renderer = ren;
}

// resource generator related functions
// Loads an image from file_loc, converts it to an SDL_Texture, and returns it; caller is responsible for destroying the texture.
SDL_Texture *Renderer::create_image(char *file_loc)
{
    SDL_Surface *img;
    // allows for loading of various image types
    img = IMG_Load(file_loc);
    if (!img) {
        printf("Error in loading bmp file %s, %s\n", file_loc, SDL_GetError());
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, img);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_DestroySurface(img);
    if (!tex) {
        printf("Error in creating bmp texture, %s\n", SDL_GetError());
    }

    return tex;
}

// Loads a GIF/animation from file_loc, writes frame count to n_frames and inter-frame delay to delay, and returns a heap array of SDL_Texture pointers for each frame.
SDL_Texture **Renderer::create_animation(char *file_loc, int *n_frames, int *delay)
{
    IMG_Animation *anim = IMG_LoadAnimation(file_loc);
    if (!anim)
    {
        printf("Failed to load animation, %s\n", SDL_GetError());
    }
    SDL_Texture **textures = (SDL_Texture **)malloc(sizeof(SDL_Texture *) * anim->count);

    for (size_t i = 0; i < anim->count; i++)
    {
        textures[i] = SDL_CreateTextureFromSurface(renderer, anim->frames[i]);
    }
    *n_frames = anim->count;
    *delay = anim->delays[0];

    IMG_FreeAnimation(anim);

    return textures;
}

// start menu related functions
// Loads the button texture and text, assigns the appropriate callback based on the button's choice_number, and stores it in choice_buttons.
void Renderer::add_startmenu_button(Button button)
{
    SDL_Texture *b_texture = create_image((char *)"assets/sample_button.bmp");

    button.set_button_texture(b_texture);
    button.set_text_texture(renderer, font, SDL_Color{255, 255, 255, 255});

    if (button.get_choice_number() == 0) // start button
    {
        button.set_callback(
            start_button_callback
        );
        choice_buttons[0] = button;
    } else
    if (button.get_choice_number() == 1) // continue button
    {
        button.set_callback(
            continue_button_callback
        );

        windows[CONTINUE_WINDOW_IDX] = Window(
            show_text((char *)button.get_text(), false).text,
            (SDL_FRect){.x = 50, .y = 50, .w = (float)(window_w-100), .h = (float)(window_h-100)},
            (SDL_Color){.r=100, .g=100, .b=255, .a=255},
            NULL, NULL, 0, NULL, 0, NULL, 0
        );
        n_windows++;

        choice_buttons[1] = button;
    } else
    if (button.get_choice_number() == 2) // settings button
    {
        button.set_callback(
            settings_button_callback
        );
        Label labels[] = {
            Label(show_text((char *)"BGM", false).text, SDL_FRect{.x=10, .y=10})
        };
        Slider sliders[] = {
            Slider((const char *)"Volume", SDL_FRect{.x=10, .y=30}, 0, 50, 25)
        };

        windows[SETTINGS_WINDOW_IDX] = Window(
            show_text((char *)button.get_text(), false).text,
            (SDL_FRect){.x = 50, .y = 50, .w = (float)(window_w-100), .h = (float)(window_h-100)},
            (SDL_Color){.r=100, .g=100, .b=255, .a=255},
            NULL, labels, TRUE_ARRAY_LEN(labels), NULL, 0, NULL, 0
        );
        n_windows++;

        choice_buttons[2] = button;
    } else
    if (button.get_choice_number() == 3) // exit button
    {
        button.set_callback(
            exit_button_callback
        );
        choice_buttons[3] = button;
    }
}

// sprite related functions
// Appends a Sprite to the sprites array, printing a warning and returning early if the array is full.
void Renderer::add_sprite(Sprite sprite)
{
    if (n_sprites == MAX_SPRITE_AMOUNT) { printf("max sprites amount reached\n"); return; }
    sprites[n_sprites++] = sprite;
}

// Wraps a Sprite in a Texture and adds it to scene_objects with given position/scale, fade speed, move speed, and target x.
int Renderer::add_sprite_to_scene(Sprite sprite, SDL_FRect pos_and_scale, float fade_speed, float move_speed, float move_to_x, int sprite_idx)
{
    if (n_scene_objects == MAX_SCENE_OBJECTS) { printf("max objects in scene\n"); return -1; }

    Texture t = {};
    if (pos_and_scale.w != 1 || pos_and_scale.h != 1)
    {
        SDL_SetTextureScaleMode(sprite.texture, SDL_SCALEMODE_NEAREST);
    }

    t.texture = sprite.texture;
    strncpy(t.identifier, sprite.name, 63); t.identifier[63] = '\0';
    t.type = TEXTURE_TYPE_IMAGE;
    t.sprite_type = sprite.type;
    t.transformation = pos_and_scale;
    t.alpha = 0.0f;
    t.fade_speed = fade_speed;
    t.fade_dir = 1;
    t.speed = move_speed;
    t.move_to_x = move_to_x;
    t.sprite_idx = sprite_idx;

    scene_objects[n_scene_objects++] = t;

    return (n_scene_objects-1);
}

// Marks any scene_object whose identifier matches name as empty and clears its texture and type fields.
void Renderer::remove_sprite_from_scene(char *name)
{
    for (size_t i = 0; i < n_scene_objects; i++)
    {
        if (!strcmp(scene_objects[i].identifier, name))
        {
            scene_objects[i].sprite_type = EMPTY_SPRITE;
            memset(scene_objects[i].identifier, 0, sizeof(scene_objects[i].identifier));
            scene_objects[i].texture = NULL; scene_objects[i].type = TEXTURE_EMPTY;
        }
    }
}

// Loads an image from image_filepath, constructs a named Sprite of the given type, and adds it to the sprites array.
void Renderer::create_sprite(char *name, char *image_filepath, Sprite_Type sprite_type)
{
    SDL_Texture *tex = create_image(image_filepath);
    Sprite s = {};
    strncpy(s.name, name, 63); s.name[63] = '\0'; s.texture = tex;
    s.type = sprite_type;
    add_sprite(s);
}

// Applies the current alpha to obj's texture and increments alpha by fade_speed each frame, removing the sprite from the scene once it has fully faded out.
void Renderer::fade_sprite(Texture &obj) // this handles sprites fading in and out
{
    SDL_SetTextureAlphaMod(obj.texture, obj.alpha);

    if (obj.alpha <= 255 && obj.alpha >= 0) {
        obj.alpha += obj.fade_speed * obj.fade_dir;
    }
    if (obj.alpha > 255) {
        obj.alpha = 255;
    }
    if (obj.alpha <= 0 && obj.fade_dir == -1) {
        obj.alpha = 0;
        remove_sprite_from_scene(obj.identifier);
    }
}

// Moves obj's x position toward finalx each frame by obj.speed, clamping once the target is reached.
void Renderer::move_sprite(Texture &obj, float finalx)
{
    if ((obj.speed > 0) && obj.transformation.x <= finalx)
    {
        should_pause = true;
        obj.transformation.x += obj.speed;
    } else
    if ((obj.speed < 0) && obj.transformation.x >= finalx)
    {
        should_pause = true;
        obj.transformation.x += obj.speed;
    }

    if ((obj.speed > 0) && obj.transformation.x > finalx)
    {
        obj.transformation.x = finalx;
        obj.speed = 0;
        should_pause = false;
    } else
    if ((obj.speed < 0) && obj.transformation.x < finalx)
    {
        obj.transformation.x = finalx;
        obj.speed = 0;
        should_pause = false;
    }
}

// Opens a TTF font from font_file at size 16 and loads the text box background image from text_box_image, setting the text box position.
void Renderer::create_text_box(const char *text_box_image, const char *font_file)
{
    font = TTF_OpenFont(font_file, 16);
    if (!font) {
        printf("Error in creating font, %s\n", SDL_GetError());
    }
    text_box_texture = create_image((char *)text_box_image);

    /*
    text_box.w = 400; text_box.h = 60;
    */
    text_box.x = TEXT_BOX_POS_X; text_box.y = TEXT_BOX_POS_Y;
}

// Renders text to a white SDL_Texture with a black drop-shadow and stores the result in text_texture.
text_texture_ret Renderer::show_text(char *text, bool for_textbox)
{
    text_texture_ret t_texture = {0};

    char *sample_text = (char *)malloc(sizeof(char) * TEXT_BOX_CHARS);
    char *speaker_text = NULL;
    if (for_textbox)
    {
        speaker_text = (char *)malloc(sizeof(char) * 128);
        strncpy(speaker_text, text, strlen(text));
        speaker_text[strlen(text)] = '\0';

        strncpy(sample_text, text + strlen(text) + 1, strlen(text + strlen(text)+1));
        sample_text[strlen(text + strlen(text) + 1)] = '\0';
    } else {
        strncpy(sample_text, text, strlen(text));
        sample_text[strlen(text)] = '\0';
    }

    // this just creates the text texture and also a shadow
    int wrap_w = 0;
    if (for_textbox)
    {
        // copy text to text box
        strncpy(full_text, text, strlen(text));
        full_text[strlen(text)] = '\0';

        float tbw, tbh;
        SDL_GetTextureSize(text_box_texture, &tbw, &tbh);
        wrap_w = (int)(tbw - TEXT_BOX_PADDING * 2);
    }

    SDL_Color text_color;
    text_color.r=255;text_color.g=255;text_color.b=255;text_color.a=255;
    SDL_Surface *shadow_surface = TTF_RenderText_Blended_Wrapped(font, sample_text, 0, text_color, wrap_w);
    text_color.r=0;text_color.g=0;text_color.b=0;text_color.a=255;
    SDL_Surface *text_surface = TTF_RenderText_Blended_Wrapped(font, sample_text, 0, text_color, wrap_w);

    int shadow_offset = -2;

    SDL_Rect blit_rect = SDL_Rect{shadow_offset, shadow_offset, shadow_surface->w, shadow_surface->h};
    SDL_BlitSurface(shadow_surface, NULL, text_surface, &blit_rect);

    if (!text_surface) {
        printf("Error in creating text surface\n");
    }
    t_texture.text = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (!t_texture.text) {
        printf("Error in creating text texture\n");
    }

    if (speaker_text != NULL)
    {
        SDL_Color text_color;
        text_color.r=255;text_color.g=255;text_color.b=255;text_color.a=255;
        SDL_Surface *shadow_surface = TTF_RenderText_Blended_Wrapped(font, speaker_text, 0, text_color, wrap_w);
        text_color.r=0;text_color.g=0;text_color.b=0;text_color.a=255;
        SDL_Surface *text_surface = TTF_RenderText_Blended_Wrapped(font, speaker_text, 0, text_color, wrap_w);

        int shadow_offset = -2;

        SDL_Rect blit_rect = SDL_Rect{shadow_offset, shadow_offset, shadow_surface->w, shadow_surface->h};
        SDL_BlitSurface(shadow_surface, NULL, text_surface, &blit_rect);

        if (!text_surface) {
            printf("Error in creating text surface\n");
        }
        t_texture.speaker = SDL_CreateTextureFromSurface(renderer, text_surface);
        if (!t_texture.speaker) {
            printf("Error in creating text texture\n");
        }

        SDL_DestroySurface(text_surface);
        SDL_DestroySurface(shadow_surface);
    }

    SDL_DestroySurface(text_surface);
    SDL_DestroySurface(shadow_surface);

    free(speaker_text);
    free(sample_text);
    
    return t_texture;
}

// Parses a comma-separated choices_csv string and creates n_choices labelled Button objects, storing them in choice_buttons.
void Renderer::show_choices(int n_choices, char *choices_csv)
{
    Button *choices_buttons = (Button*)malloc(sizeof(Button) * n_choices);

    char *choice;
    choice = strtok(choices_csv, ",");

    SDL_Texture *b_texture = create_image((char *)"assets/sample_button.bmp");

    int i = 0;
    while (choice != NULL)
    {
        Button b = Button(choice, i+1, SDL_FRect{0, (float)(CHOICES_BUTTON_START+(i*CHOICES_BUTTON_MARGIN)), (float)window_w, 25}, true);
        b.set_button_texture(b_texture);
        b.set_text_texture(renderer, font, SDL_Color{255, 255, 255, 255});

        choices_buttons[i++] = b;

        choice = strtok(NULL, ",");
    }

    choice_buttons = choices_buttons;
    n_choice_buttons = n_choices;
}

// Runs the main event-poll/update/render loop at the given fps, dispatching handler tokens each frame and drawing all scene objects,
// text, animations, and buttons until the game ends.
void Renderer::main_graphics_loop(int fps, Handler &handler)
{
    //Window w = Window(show_text((char *)" Options", false), (SDL_FRect){.x=50, .y=50, .w=200, .h=200}, (SDL_Color){.r=100, .g=100, .b=255, .a=255}, NULL);
    SDL_Event event;
    // used in storing current Texture obj from scene array
    float x, y; // used in getting texture size
    Handler_Return handler_ret;
    running = true;
    Uint64 anim_start_time = SDL_GetTicks();

    float mouse_x, mouse_y;

    while (running) {
        SDL_GetMouseState(&mouse_x, &mouse_y);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            } else
            if (((event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE) ||
                (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)) && skippable_pause)
            {
                audio_engine.stop_wav(btn_click_audio_id, 1.0f);
                audio_engine.play_wav(btn_click_audio_id, 1.0f, 1.0f);
                if (should_pause && n_choice_buttons == 0)
                {
                    should_pause = false;
                }
            }
            
            if (show_window && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                SDL_FRect dims = windows[window_to_render].get_dims();
                if (mouse_x > (dims.x+dims.w) || mouse_x < (dims.x) || mouse_y > (dims.y+dims.h) || mouse_y < (dims.y))
                {
                    window_to_render = -1;
                    show_window = false;
                }
            } else
            // debugging related inputs
            if ((event.type == SDL_EVENT_KEY_DOWN) && event.key.key == SDLK_A)
            {
                print_sprites();
            } else
            if ((event.type == SDL_EVENT_KEY_DOWN) && event.key.key == SDLK_D)
            {
                print_textures();
            } else
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT)
            {
                bool choice_was_chosen = false;
                for (size_t i = 0; i < n_choice_buttons; i++)
                {
                    if (choice_buttons[i].check_button_hovered(mouse_x, mouse_y))
                    {
                        if (choice_buttons[i].is_button_choice())
                        {
                            handler.set_choice_chosen(choice_buttons[i].get_choice_number());
                            should_pause = false;
                            choice_was_chosen = true;
                        } else
                        {
                            choice_buttons[i].call_callback();
                        }
                    }
                }

                if (choice_was_chosen)
                {
                    choice_buttons = NULL;
                    n_choice_buttons = 0;
                }
            }
        }

        if (!should_pause)
        {
            handler_ret = handler.handle_next_token();

            if (handler_ret.command == SHOW_STARTMENU)
            {
                choice_buttons = (Button *)malloc(sizeof(Button) * 4);
                n_choice_buttons += 4;
            } else
            if (handler_ret.command == MENU_BUTTON_ADJUSTMENT)
            {
                Button b = Button(handler_ret.string_data, handler_ret.integer_data,
                    SDL_FRect{.x=handler_ret.float_array[0], .y=handler_ret.float_array[1],
                              .w=handler_ret.float_array[2], .h=handler_ret.float_array[3]}, false);
                add_startmenu_button(b);
            } else
            if (handler_ret.command == ADD_SPRITE_TO_ARRAY)
            {
                create_sprite(handler_ret.sprite_data.name, handler_ret.string_data, handler_ret.sprite_data.type);
            } else
            if (handler_ret.command == ADD_SPRITE_TO_SCENE)
            {
                add_sprite_to_scene(sprites[handler_ret.integer_data],
                    SDL_FRect{
                    .x=handler_ret.float_array[FLOATARRAY_POSX],
                    .y=handler_ret.float_array[FLOATARRAY_POSY],
                    .w=handler_ret.float_array[FLOATARRAY_SCALEX],
                    .h=handler_ret.float_array[FLOATARRAY_SCALEY]},
                    handler_ret.float_array[FLOATARRAY_FADE], handler_ret.float_array[FLOATARRAY_SPEED], handler_ret.float_array[FLOATARRAY_MOVETOX], handler_ret.integer_data);
            } else
            if (handler_ret.command == REMOVE_SPRITE_FROM_SCENE)
            {
                for (size_t i = 0; i < n_scene_objects; i++)
                {
                    if (!strcmp(sprites[handler_ret.integer_data].name, scene_objects[i].identifier))
                    {
                        scene_objects[i].fade_dir = -1;
                        scene_objects[i].fade_speed = handler_ret.float_array[FLOATARRAY_FADE];
                    }
                }
            } else
            if (handler_ret.command == MOVE_SPRITE)
            {
                for (size_t i = 0; i < n_scene_objects; i++)
                {
                    if (!strcmp(sprites[handler_ret.integer_data].name, scene_objects[i].identifier))
                    {
                        scene_objects[i].speed = handler_ret.float_array[FLOATARRAY_SPEED];
                        scene_objects[i].move_to_x = handler_ret.float_array[FLOATARRAY_POSX];
                    }
                }
            } else
            if (handler_ret.command == CROSSFADE_SPRITE)
            {
                for (size_t i = 0; i < n_scene_objects; i++)
                {
                    if (!strcmp(scene_objects[i].identifier, sprites[handler_ret.integer_data].name))
                    {
                        handler_ret.integer_data = i;
                        break;
                    }
                }

                for (size_t i = 0; i < n_sprites; i++)
                {
                    if (!strcmp(sprites[i].name, handler_ret.string_data))
                    {
                        int dest = add_sprite_to_scene(
                            sprites[i],
                            scene_objects[handler_ret.integer_data].transformation,
                            handler_ret.float_array[FLOATARRAY_FADE],
                            handler_ret.float_array[FLOATARRAY_SPEED],
                            scene_objects[handler_ret.integer_data].move_to_x,
                            i
                        );

                        scene_objects[handler_ret.integer_data].fade_dir = -1;
                        scene_objects[handler_ret.integer_data].fade_speed = handler_ret.float_array[FLOATARRAY_FADE];
                        scene_objects[dest].fade_dir = 1;
                        scene_objects[dest].fade_speed = handler_ret.float_array[FLOATARRAY_FADE];
                        break;
                    }
                }
            } else
            if (handler_ret.command == CREATE_ANIMATION)
            {
                current_animation_frames = create_animation(handler_ret.string_data, &n_current_animation_frames, &delay_bw_frames);
            } else
            if (handler_ret.command == PLAY_ANIMATION)
            {
                play_animation = true;
            } else
            if (handler_ret.command == STOP_ANIMATION)
            {
                play_animation = false;
            } else
            if (handler_ret.command == LOAD_AUDIO)
            {
                audio_engine.load_wav(handler_ret.integer_data, handler_ret.string_data);
            } else
            if (handler_ret.command == PLAY_AUDIO)
            {
                audio_engine.play_wav(handler_ret.integer_data, handler_ret.float_array[FLOATARRAY_VOL], handler_ret.float_array[FLOATARRAY_FADE]);
            } else
            if (handler_ret.command == STOP_AUDIO)
            {
                audio_engine.stop_wav(handler_ret.integer_data, handler_ret.float_array[FLOATARRAY_FADE]);
            } else
            if (handler_ret.command == SHOW_TEXT)
            {
                text_texture_ret sp_and_txt = show_text(handler_ret.string_data, true);
                speaker_texture = sp_and_txt.speaker;
                text_texture = sp_and_txt.text;
            } else
            if (handler_ret.command == CLEAR_TEXT)
            {
                memset(full_text, 0, strlen(full_text));
            } else
            if (handler_ret.command == WAITFOR_USERINPUT)
            {
                skippable_pause = true;
                should_pause = true;
            } else
            if (handler_ret.command == WAITFOR_DELAY)
            {
                //SDL_Delay(handler_ret.integer_data);
                skippable_pause = false;
                should_pause = true;
                SDL_AddTimer(handler_ret.integer_data, delay_timer_callback, NULL);
            } else
            if (handler_ret.command == WAITFOR_BUTTONCALLBACK)
            {
                should_pause = true;
            } else
            if (handler_ret.command == SHOW_CHOICES)
            {
                show_choices(handler_ret.integer_data, handler_ret.string_data);
            } else
            if (handler_ret.command == SET_BUTTON_PARAM)
            {
                if (handler_ret.float_array[0] == 0 && handler_ret.float_array[1] == 1)
                {
                    btn_hover_audio_id = handler_ret.integer_data;
                } else
                if (handler_ret.float_array[0] == 1 && handler_ret.float_array[1] == 0)
                {
                    btn_click_audio_id = handler_ret.integer_data;
                }
            } else
            if (handler_ret.command == END_OF_GAME)
            {
                running = false;
            } else
            if (handler_ret.command == CONTINUE) { }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // render scene
        // play any animation
        if (play_animation)
        {
            Uint64 now = SDL_GetTicks();
            SDL_RenderTexture(renderer, current_animation_frames[current_frame], NULL, NULL);

            if ((now - anim_start_time) >= delay_bw_frames)
            {
                current_frame ++;
                if (current_frame >= n_current_animation_frames)
                {
                    current_frame = 0;
                }
                anim_start_time = now;
            }
        }

        // render sprites and background
        for (size_t i = 0; i < n_scene_objects; i++)
        {
            Texture &obj = scene_objects[i];
            if (obj.type != TEXTURE_TYPE_TEXT) {

                if (obj.sprite_type == BACKGROUND_SPRITE)
                {
                    SDL_GetTextureSize(obj.texture, &x, &y);

                    fade_sprite(obj);

                    float scale = (window_w / x > window_h / y)
                        ? (window_w / x)
                        : (window_h / y);

                    SDL_FRect dst;
                    dst.w = x * scale;
                    dst.h = y * scale;
                    // center it (important for cropping effect)
                    dst.x = (window_w - dst.w) / 2;
                    dst.y = (window_h - dst.h) / 2;

                    SDL_RenderTexture(renderer, obj.texture, NULL, &dst);
                } else
                if (obj.sprite_type == CHARACTER_SPRITE) {
                    SDL_GetTextureSize(obj.texture, &x, &y);

                    fade_sprite(obj);
                    move_sprite(obj, obj.move_to_x);

                    SDL_FRect dst = {
                        (float)(0+obj.transformation.x), (float)(window_h-y+obj.transformation.y),
                        x*obj.transformation.w, y*obj.transformation.h
                    }; // h-y makes it stick to bottom

                    SDL_RenderTexture(renderer, obj.texture, NULL, &dst);
                } else
                if (obj.sprite_type == EMPTY_SPRITE || obj.type == TEXTURE_EMPTY) { continue; }
            }
        }

        // render text
        if (full_text && (strcmp(full_text, "") || cur_tb_h > 0.0))
        {
            // render text box
            if (text_box_texture != NULL) {
                SDL_FRect rect = {.x = 140, .y = 530};
                SDL_GetTextureSize(text_box_texture, &x, &y);
                rect.w = x, rect.h = cur_tb_h;

                if (cur_tb_h < y && strcmp(full_text, "")) { cur_tb_h+=tb_anim_speed; }
                else if (!strcmp(full_text, "")) { cur_tb_h-=tb_anim_speed; }

                SDL_RenderTexture(renderer, text_box_texture, NULL, &rect);
            }

            SDL_GetTextureSize(text_texture, &x, &y);

            // render actual text
            text_box.w = x; text_box.h = y; text_box.x = 140 + TEXT_BOX_PADDING;
            SDL_RenderTexture(renderer, text_texture, NULL, &text_box);
        }

        // render any ui or mid story choice buttons
        for (size_t i = 0; i < n_choice_buttons; i++)
        {
            choice_buttons[i].render_button(renderer);
            bool hovered = choice_buttons[i].check_button_hovered(mouse_x, mouse_y);

            if (hovered && !choice_buttons[i].hovered_last_frame)
            {
                if (btn_hover_audio_id)
                {
                    audio_engine.stop_wav(btn_hover_audio_id, 1.0f);
                    audio_engine.play_wav(btn_hover_audio_id, 1.0f, 1.0f);

                    choice_buttons[i].hovered_last_frame = true;
                }
            } else
            if (!hovered && choice_buttons[i].hovered_last_frame) {
                choice_buttons[i].hovered_last_frame = false;
            }
        }

        if (show_window)
        {
            windows[window_to_render].render_window(renderer);
            // better to render close button here since all logic is here
        }

        audio_engine.update_audios();
        SDL_RenderPresent(renderer);

        // delay for capping fps
        if (!play_animation)
        {
            SDL_Delay(1000/fps);
        }
    }

    SDL_Quit();
}

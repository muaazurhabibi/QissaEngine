#include "handler.hpp"

/*
    Handler handles all input tokens; it looks at input user gave in the script, and sends renderer corresponding instruction
*/

// Constructs the Handler, storing the tokens array pointer, its length, and initialising the current token index to 0.
Handler::Handler(Token *tokens, int tokens_len) : tokens_array(tokens)
{
    tokens_array = tokens;
    n_tokens = tokens_len;
    cur_token = 0;
}

// Reads and advances past the current token, translates its command string into a Handler_Return
// with the appropriate command enum and associated data, and returns it to the renderer.
Handler_Return Handler::handle_next_token()
{
    Handler_Return ret = {};

    if (cur_token >= n_tokens)
    {
        ret.command = CONTINUE;
        return ret;
    }
    Token tok;
    tok = tokens_array[cur_token];
    cur_token++;

    ret.token_data = tok;

    if (!strcmp(tok.command, ""))
    {
        return ret;
    }

    if (!strcmp(tok.command, "char") || !strcmp(tok.command, "bg")) // creates a new char or bg
    {
        ret.command = ADD_SPRITE_TO_ARRAY;
        ret.sprite_data = Sprite{};
        if (!strcmp(tok.command, "char"))
        {
            ret.sprite_data.type = CHARACTER_SPRITE;
        } else
        if (!strcmp(tok.command, "bg"))
        {
            ret.sprite_data.type = BACKGROUND_SPRITE;
        }

        strncpy(ret.sprite_data.name, tok.opcode, 63);
        ret.sprite_data.name[63] = '\0';
        strncpy(ret.string_data, tok.additional_params, strlen(tok.additional_params));
        ret.string_data[strlen(tok.additional_params)+1] = '\0';

        ret.sprite_data.texture = NULL;

        sprites[n_sprites++] = ret.sprite_data;
        return ret;
    } else
    if (!strcmp(tok.command, "anim"))
    {
        ret.command = CREATE_ANIMATION;
        strncpy(ret.string_data, tok.additional_params, strlen(tok.additional_params));
        ret.string_data[strlen(tok.additional_params)] = '\0';

        strncpy(ret.sprite_data.name, tok.opcode, strlen(tok.opcode));
        ret.sprite_data.name[strlen(tok.opcode)] = '\0';

        return ret;
    } else
    if (!strcmp(tok.command, "playanim"))
    {
        // this currently assumes that we can only play one animation at the moment
        ret.command = PLAY_ANIMATION;
        return ret;
    } else
    if (!strcmp(tok.command, "stopanim"))
    {
        ret.command = STOP_ANIMATION;
        return ret;
    } else
    if (!strcmp(tok.command, "showchar") || !strcmp(tok.command, "showbg")) // shows the char or bg
    {
        if (!strcmp(tok.command, "showchar"))
        {
            int i = 0;
            char *token = strtok(tok.additional_params, ",");

            while (token != NULL)
            {
                if (i >= sizeof(ret.float_array)/sizeof(float))
                {
                    printf("cant continue operation, ran out of integer array space\n");
                    break;
                }

                char to_int[4];
                if (!strncmp(token, "posx=", 5))
                {
                    strncpy(to_int, token+5, strlen(token));
                    ret.float_array[FLOATARRAY_POSX] = atof(to_int);
                } else
                if (!strncmp(token, "posy=", 5))
                {
                    strncpy(to_int, token+5, strlen(token));
                    ret.float_array[FLOATARRAY_POSY] = atof(to_int);
                }
                else
                if (!strncmp(token, "scalex=", 7))
                {
                    strncpy(to_int, token+7, strlen(token));
                    ret.float_array[FLOATARRAY_SCALEX] = atof(to_int);
                }
                else
                if (!strncmp(token, "scaley=", 7))
                {
                    strncpy(to_int, token+7, strlen(token));
                    ret.float_array[FLOATARRAY_SCALEY] = atof(to_int);
                } else
                if (!strncmp(token, "fade=", 5))
                {
                    strncpy(to_int, token+5, strlen(token));
                    ret.float_array[FLOATARRAY_FADE] = atof(to_int);
                } else
                if (!strncmp(token, "speed=", 6))
                {
                    strncpy(to_int, token+6, strlen(token));
                    ret.float_array[FLOATARRAY_SPEED] = atof(to_int);
                } else
                if (!strncmp(token, "movetox=", 8))
                {
                    strncpy(to_int, token+8, strlen(token));
                    ret.float_array[FLOATARRAY_MOVETOX] = atof(to_int);
                }

                token = strtok(NULL, ",");
            }
        } else
        if (!strcmp(tok.command, "showbg"))
        {
            ret.float_array[FLOATARRAY_FADE] = atof(tok.additional_params);
        }

        for (size_t i = 0; i < n_sprites; i++)
        {
            if (!strcmp(sprites[i].name, tok.opcode)) {
                printf("Adding %s to scene\n", sprites[i].name);
                ret.command = ADD_SPRITE_TO_SCENE;
                ret.integer_data = i;
                return ret;
            }
        }
    } else
    if (!strcmp(tok.command, "hidechar") || !strcmp(tok.command, "hidebg")) // hides a character or background
    {
        ret.command = REMOVE_SPRITE_FROM_SCENE;
        size_t i = 0;
        char *token = strtok(tok.additional_params, ",");

        while (token != NULL)
        {
            if (i >= sizeof(ret.float_array)/sizeof(float))
            {
                printf("cant continue operation, ran out of integer array space\n");
                break;
            }

            char to_int[4];
            if (!strncmp(token, "fade=", 5))
            {
                strncpy(to_int, token+5, strlen(token));
                ret.float_array[FLOATARRAY_FADE] = atof(to_int);
            }

            token = strtok(NULL, ",");
        }

        for (size_t i = 0; i < n_sprites; i++)
        {
            if (!strcmp(sprites[i].name, tok.opcode))
            {
                ret.integer_data = i;
                return ret;
            }
        }
    } else
    if (!strcmp(tok.command, "movechar"))
    {
        ret.command = MOVE_SPRITE;
        size_t i = 0;
        char *token = strtok(tok.additional_params, ",");

        while (token != NULL)
        {
            if (i >= sizeof(ret.float_array)/sizeof(float))
            {
                printf("cant continue operation, ran out of integer array space\n");
                break;
            }

            char to_int[4];
            if (!strncmp(token, "posx=", 5))
            {
                strncpy(to_int, token+5, strlen(token));
                ret.float_array[FLOATARRAY_POSX] = atof(to_int);
            } else
            if (!strncmp(token, "posy=", 5))
            {
                strncpy(to_int, token+5, strlen(token));
                ret.float_array[FLOATARRAY_POSY] = atof(to_int);
            }else
            if (!strncmp(token, "speed=", 6))
            {
                strncpy(to_int, token+6, strlen(token));
                ret.float_array[FLOATARRAY_SPEED] = atof(to_int);
            }

            token = strtok(NULL, ",");
        }

        for (size_t i = 0; i < n_sprites; i++)
        {
            if (!strcmp(sprites[i].name, tok.opcode))
            {
                ret.integer_data = i;
                return ret;
            }
        }
    } else
    if (!strcmp(tok.command, "crossfade"))
    {
        ret.command = CROSSFADE_SPRITE;
        size_t i = 0;
        char *token = strtok(tok.additional_params, ",");

        while (token != NULL)
        {
            if (i >= sizeof(ret.float_array)/sizeof(float))
            {
                printf("cant continue operation, ran out of integer array space\n");
                break;
            }

            char to_int[4];
            if (!strncmp(token, "dest=", 5))
            {
                strncpy(ret.string_data, token+5, strlen(token));
            } else
            if (!strncmp(token, "fade=", 5))
            {
                strncpy(to_int, token+5, strlen(token));
                ret.float_array[FLOATARRAY_FADE] = atof(to_int);
            } else
            if (!strncmp(token, "speed=", 6))
            {
                strncpy(to_int, token+6, strlen(token));
                ret.float_array[FLOATARRAY_SPEED] = atof(to_int);
            }


            token = strtok(NULL, ",");
        }

        for (size_t i = 0; i < n_sprites; i++)
        {
            if (!strcmp(sprites[i].name, tok.opcode))
            {
                ret.integer_data = i;
                return ret;
            }
        }
    } else
    if (!strcmp(tok.command, "showtext")) // shows text
    {
        ret.command = SHOW_TEXT;
        strncpy(ret.string_data, tok.opcode, strlen(tok.opcode)); ret.string_data[strlen(tok.opcode)] = '\0';
        strncpy(ret.string_data + strlen(ret.string_data) + 1, tok.additional_params, strlen(tok.additional_params));
        ret.string_data[strlen(tok.additional_params) + strlen(tok.opcode) + 1] = '\0';
        printf("%s\n", ret.string_data);
        printf("%s\n", ret.string_data+strlen(ret.string_data)+1);
        return ret;
    } else
    if (!strcmp(tok.command, "cleartext")) // clears text
    {
        ret.command = CLEAR_TEXT;
        return ret;
    } else
    if (!strcmp(tok.command, "audio"))
    {
        ret.command = LOAD_AUDIO;
        strncpy(ret.string_data, tok.additional_params, strlen(tok.additional_params)); ret.string_data[strlen(tok.additional_params)] = '\0';
        ret.integer_data = atoi(tok.opcode);
        return ret;
    } else
    if (!strcmp(tok.command, "playaudio"))
    {
        ret.command = PLAY_AUDIO;
        ret.integer_data = atoi(tok.opcode);

        char *token = strtok(tok.additional_params, ",");
        while (token != NULL)
        {
            char to_int[4];
            if (!strncmp(token, "vol=", 4))
            {
                strncpy(to_int, token+4, strlen(token));
                ret.float_array[FLOATARRAY_VOL] = atof(to_int);
            } else
            if (!strncmp(token, "fade=", 5))
            {
                strncpy(to_int, token+5, strlen(token));
                ret.float_array[FLOATARRAY_FADE] = atof(to_int);
            }

            token = strtok(NULL, ",");
        }

        return ret;
    } else
    if (!strcmp(tok.command, "stopaudio"))
    {
        ret.command = STOP_AUDIO;
        ret.integer_data = atoi(tok.opcode);

        char *token = strtok(tok.additional_params, ",");
        while (token != NULL)
        {
            char to_int[4];
            if (!strncmp(token, "fade=", 5))
            {
                strncpy(to_int, token+5, strlen(token));
                ret.float_array[FLOATARRAY_FADE] = atof(to_int);
            }

            token = strtok(NULL, ",");
        }

        return ret;
    }
    if (!strcmp(tok.command, "waitfor")) // waits for a command
    {
        if (!strcmp(tok.opcode, "advance")) {
            ret.command = WAITFOR_USERINPUT;
            return ret;
        } else
        if (!strcmp(tok.opcode, "delay"))
        {
            ret.command = WAITFOR_DELAY;
            ret.integer_data = atoi(tok.additional_params);
        } else
        if (!strcmp(tok.opcode, "choice"))
        {
            ret.command = WAITFOR_USERINPUT;
            return ret;
        } else
        if (!strcmp(tok.opcode, "buttonevent"))
        {
            ret.command = WAITFOR_BUTTONCALLBACK;
            return ret;
        }

    } else
    if (!strcmp(tok.command, "choices")) // shows choices to user
    {
        ret.command = SHOW_CHOICES;
        ret.integer_data = atoi(tok.opcode);
        strncpy(ret.string_data, tok.additional_params, strlen(tok.additional_params)); ret.string_data[strlen(tok.additional_params)] = '\0';

        return ret;
    } else
    if (!strcmp(tok.command, "case")) // handle a choice that the user made
    {
        if (choice_chosen > 0 && choice_chosen == atoi(tok.opcode))
        {
            char *symbol = strtok(tok.additional_params, ":");
            symbol = strtok(NULL, ":");
            cur_token = search_for_symbol(symbol);
        }
    } else
    if (!strcmp(tok.command, "startmenu"))
    {
        ret.command = SHOW_STARTMENU;
        return ret;
    } else
    if (!strcmp(tok.command, "playbutton") ||
    !strcmp(tok.command, "continuebutton") ||
    !strcmp(tok.command, "settingsbutton") ||
    !strcmp(tok.command, "quitbutton")) // set params of start menu buttons
    {
        ret.command = MENU_BUTTON_ADJUSTMENT;
        strncpy(ret.string_data, tok.opcode, strlen(tok.opcode)); ret.string_data[strlen(tok.opcode)] = '\0';

        if (!strcmp(tok.command, "playbutton"))
        {
            ret.integer_data = 0;
        } else
        if (!strcmp(tok.command, "continuebutton"))
        {
            ret.integer_data = 1;
        } else
        if (!strcmp(tok.command, "settingsbutton"))
        {
            ret.integer_data = 2;
        } else
        if (!strcmp(tok.command, "quitbutton"))
        {
            ret.integer_data = 3;
        }

        int i = 0;
        char *token = strtok(tok.additional_params, ",");

        while (token != NULL)
        {
            if (i >= sizeof(ret.float_array)/sizeof(float))
            {
                printf("cant continue operation, ran out of integer array space\n");
                break;
            }

            char to_int[4];
            if (!strncmp(token, "posx=", 5))
            {
                strncpy(to_int, token+5, strlen(token));
                ret.float_array[FLOATARRAY_POSX] = atof(to_int);
            } else
            if (!strncmp(token, "posy=", 5))
            {
                strncpy(to_int, token+5, strlen(token));
                ret.float_array[FLOATARRAY_POSY] = atof(to_int);
            }
            else
            if (!strncmp(token, "width=", 6))
            {
                strncpy(to_int, token+6, strlen(token));
                ret.float_array[2] = atof(to_int);
            }
            else
            if (!strncmp(token, "height=", 7))
            {
                strncpy(to_int, token+7, strlen(token));
                ret.float_array[3] = atof(to_int);
            }
            //ret.float_array[i++] = atof(to_int);

            token = strtok(NULL, ",");
        }

        return ret;
    } else
    if (!strcmp(tok.command, "btn"))
    {
        ret.command = SET_BUTTON_PARAM;
        if (!strcmp(tok.opcode, "hvr"))
        {
            ret.float_array[0] = 0;
            ret.float_array[1] = 1;
        } else
        if (!strcmp(tok.opcode, "clk"))
        {
            ret.float_array[0] = 1;
            ret.float_array[1] = 0;
        }

        char *token = strtok(tok.additional_params, ",");

        while (token != NULL)
        {
            char to_int[4];
            if (!strncmp(token, "audio=", 6))
            {
                strncpy(to_int, token+6, strlen(token));
                ret.integer_data = atoi(to_int);
                printf("ret.integer_data=%d\n", ret.integer_data);
            }

            token = strtok(NULL, ",");
        }

        return ret;
    } else
    if (!strcmp(tok.command, "end"))
    {
        ret.command = END_OF_GAME;
        return ret;
    }

    return ret;
}

// Linearly scans tokens_array for a token whose command field matches symbol and returns its index, or -1 if not found.
int Handler::search_for_symbol(char *symbol)
{
    Token tok;
    for (size_t i = 0; i < n_tokens; i++)
    {
        tok = tokens_array[i];
        if (!strcmp(tok.command, symbol))
        {
            return i;
        }
    }
    return -1;
}

// Stores the player's selected choice number so it can be matched against subsequent "case" tokens.
void Handler::set_choice_chosen(int choice)
{
    choice_chosen = choice;
}

#pragma once

#include "parser/parser.hpp"
#include <strings.h>
#include "common.hpp"

class Handler
{
    private:
        Token *tokens_array;
        int n_tokens;
        Sprite *sprites = (Sprite*)malloc(sizeof(Sprite) * MAX_SPRITE_AMOUNT);
        int n_sprites = 0;
        int cur_token;

        int choice_chosen;

    public:
        Handler(std::nullptr_t);
        Handler(Token *tokens, int tokens_len);
        Handler_Return handle_next_token();
        int search_for_symbol(char *symbol);
        void set_sprites_data(Sprite *sprites_array, int sprites_array_len);
        void set_choice_chosen(int choice);
};
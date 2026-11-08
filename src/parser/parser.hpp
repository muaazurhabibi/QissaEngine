#pragma once

#include <stdio.h>
#include <cstdlib>
#include "../common.hpp"

class Parser {
    private:
        const char *s_file;
        Token *tokens_array;
        int token_array_len;
    
    public:
        Parser(const char* script_file);
        void parse_file();
        Token *get_tokens();
        int get_tokens_array_len();

        void print_tokens(Token* tokens, int count)
        {
            // A.I generated function to print all tokens values
            printf("---- Tokens (%d) ----\n", count);

            for (int i = 0; i < count; i++)
            {
                printf("[%02d] cmd: %-10s | op: %-10s | params: %s\n",
                    i,
                    tokens[i].command,
                    tokens[i].opcode,
                    tokens[i].additional_params);
            }
            printf("---------------------\n");
        }
};


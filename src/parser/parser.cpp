#include "parser.hpp"

Parser::Parser(const char* script_file)
{
    s_file = script_file;
    parse_file();
}

void Parser::parse_file()
{
    FILE *file = fopen(s_file, "r");
    char line_buffer[256];

    int line_n = 0;token_array_len = 0;

    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL)
    {
        if (strcmp(line_buffer, ""))
        {
            token_array_len++;
        }
    }

    tokens_array = (Token*)malloc(sizeof(Token) * token_array_len);
    rewind(file);
    line_n = 0;
    
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL)
    {
        if (strcmp(line_buffer, ""))
        {
            Token tok = {};
            sscanf(line_buffer, "%31s %31s %511[^\n]", tok.command, tok.opcode, tok.additional_params);

            tokens_array[line_n] = tok;
            line_n++;
        }
    }

    fclose(file);
}

Token *Parser::get_tokens() { return tokens_array; }

int Parser::get_tokens_array_len() { return token_array_len; }
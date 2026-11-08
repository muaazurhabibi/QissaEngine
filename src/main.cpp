#include "graphics/graphics.hpp"
#include "parser/parser.hpp"

const char GAME_TITLE[] = "_";
const char SCRIPT_FILE[] = "test_script.txt";
const char TEXT_BOX_IMAGE[] = "assets/sample_text_box.bmp";
const char FONT_FILE[] = "assets/font/CascadiaMono.ttf";

int main(int argc, char *argv[])
{
    Parser parser = Parser(SCRIPT_FILE);
    // printing out tokens, can delete this later
    //parser.print_tokens(parser.get_tokens(), parser.get_tokens_array_len());

    Renderer renderer = Renderer(1024, 768);
    renderer.init_graphics_and_audio();

    Handler handler = Handler(parser.get_tokens(), parser.get_tokens_array_len());

    renderer.create_main_window(GAME_TITLE);
    renderer.create_renderer();
    // create the text box texture
    renderer.create_text_box(TEXT_BOX_IMAGE, FONT_FILE);

    renderer.main_graphics_loop(60, handler);

    return 0;
}
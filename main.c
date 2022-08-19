#include "Board.h"

#define WIDTH 1080
#define HEIGHT 768

int main()
{
    board_load(WIDTH, HEIGHT, "Memory");

    board_loop();

    board_unload();

    return EXIT_SUCCESS;
}

//
// Created by Andrew Bowie on 4/21/23.
//

#include "mine_sweeper.h"
#include "stddef.h"
#include "stdbool.h"
#include "string.h"
#include "math.h"
#include "stdio.h"
#include "print_format.h"
#include "stdlib.h"
#include "linked_list.h"

#define MINE_WIDTH 40
#define MINE_HEIGHT 10
#define MINE_GENERATION_FACTOR 0.1

#define HORIZ_WALL "\u2501"
#define VERT_WALL "\u2503"

#define TL_CORNER "\u250F"
#define TR_CORNER "\u2513"
#define BL_CORNER "\u2517"
#define BR_CORNER "\u251B"

///The mine bitmap, 1 signifies mine, 0 signifies empty space.
static bool mine_bitmap[MINE_WIDTH][MINE_HEIGHT];
///The revealed map, 1 signifies a revealed square, 2 signifies a flagged square. You cannot reveal a flagged square.
static char revealed_map[MINE_WIDTH][MINE_HEIGHT];
///The total amount of free squares.
static int free_squares;
///The total amount of revealed squares.
static int revealed_squares;
///The total amount of mines in the current game.
static int mines;
///The total amount of mines flagged.
static int mines_flagged;
///The player's x cursor position.
static int pc_x;
///The player's y cursor position.
static int pc_y;
///If the game is currently running.
static bool game_running;

/**
 * @brief Gets the nearby mines for the location.
 *
 * @param x the x coordinate.
 * @param y the y coordinate.
 * @return the number of mines touching the space.
 * @authors Andrew Bowie
 */
int get_nearby_mines(int x, int y)
{
    int total = 0;
    for (int xNeighbor = x - 1; xNeighbor <= x + 1; ++xNeighbor)
    {
        if(xNeighbor < 0 || xNeighbor >= MINE_WIDTH)
            continue;

        for (int yNeighbor = y - 1; yNeighbor <= y + 1; ++yNeighbor)
        {
            if(yNeighbor < 0 || yNeighbor >= MINE_HEIGHT)
                continue;

            if(mine_bitmap[xNeighbor][yNeighbor])
                total++;
        }
    }
    return total;
}

/**
 * @brief Encodes the two coordinates as a single integer.
 * @param x the x coordinate.
 * @param y the y coordinate.
 * @return the encoded coordinate.
 * @authors Andrew Bowie
 */
int encode_coordinates(int x, int y)
{
    return ((x & 0xFF) << 8) | (y & 0xFF);
}

/**
 * @brief Reveals all nearby 0 spaces.
 * @authors Andrew Bowie
 */
void reveal_all_nearby(int x, int y)
{
    //Don't reveal anything more if no nearby mines.
    if(get_nearby_mines(x, y) > 0)
    {
        revealed_map[pc_x][pc_y] = 1;
        revealed_squares++;
        return;
    }

    linked_list *node = nl_unbounded();
    int full_word = encode_coordinates(x, y);

    add_item(node,  (void *) full_word);

    while(list_size(node) > 0)
    {
        int item = (int) remove_item_unsafe(node, 0);
        int x_part = (item & 0xFF00) >> 8;
        int y_part = item & 0xFF;

        if(x_part < 0 || x_part >= MINE_WIDTH || y_part < 0 || y_part >= MINE_HEIGHT)
            continue;

        //Get the nearby mines.
        bool revealed = revealed_map[x_part][y_part] == 1;
        int nearby_mines = get_nearby_mines(x_part, y_part);
        if(nearby_mines == 0 && revealed_map[x_part][y_part] == 0)
        {
            //Add neighbors.
            for (int xn = x_part - 1; xn <= x_part + 1; ++xn)
            {
                if(xn < 0 || xn >= MINE_WIDTH)
                    continue;

                for (int yn = y_part - 1; yn <= y_part + 1; ++yn)
                {
                    if(yn < 0 || yn >= MINE_HEIGHT)
                        continue;

                    add_item(node, (void *) encode_coordinates(xn, yn));
                }
            }
        }

        if(!revealed)
        {
            revealed_squares++;
            revealed_map[x_part][y_part] = 1;
        }
    }
    destroy_list(node, false);
}

/**
 * @brief Ticks the game, waiting for user input to do something.
 * @authors Andrew Bowie
 */
void ms_game_tick(void)
{
    char ch = getc();
    switch (ch)
    {
        case 'A':
        case 'a':
            pc_x = pc_x == 0 ? 0 : pc_x - 1;
            break;
        case 'W':
        case 'w':
            pc_y = pc_y == 0 ? 0 : pc_y - 1;
            break;
        case 'S':
        case 's':
            pc_y = pc_y + 1 == MINE_HEIGHT ? pc_y : pc_y + 1;
            break;
        case 'D':
        case 'd':
            pc_x = pc_x + 1 == MINE_WIDTH ? pc_x : pc_x + 1;
            break;
        case ' ':
            if(revealed_map[pc_x][pc_y] == 2)
                return;

            if(mine_bitmap[pc_x][pc_y])
                game_running = false;
            else
                reveal_all_nearby(pc_x, pc_y);
            break;
        case 'F':
        case 'f':
            if(revealed_map[pc_x][pc_y] == 2)
            {
                if(mine_bitmap[pc_x][pc_y])
                    mines_flagged--;
                revealed_map[pc_x][pc_y] = (char) 0;
            }
            else if(revealed_map[pc_x][pc_y] == 0)
            {
                if(mine_bitmap[pc_x][pc_y])
                    mines_flagged++;
                revealed_map[pc_x][pc_y] = (char) 2;
            }
            break;
    }

    if(mines == mines_flagged && revealed_squares == free_squares)
        game_running = false;
}

/**
 * @brief Prints the mine.
 * @authors Andrew Bowie
 */
void print_mine(void)
{
    clearscr();
    print(TL_CORNER);
    for (int i = 0; i < MINE_WIDTH; ++i)
    {
        print(HORIZ_WALL);
    }
    print(TR_CORNER);
    print("\n");
    for (int y = 0; y < MINE_HEIGHT; ++y)
    {
        print(VERT_WALL);
        for (int x = 0; x < MINE_WIDTH; ++x)
        {
            bool pc_pos = pc_x == x && pc_y == y;
            bool revealed = revealed_map[x][y] == 1;
            bool flagged = revealed_map[x][y] == 2;
            const color_t *clr = get_output_color();

            //Check if the player's cursor is on that position.
            if(pc_pos)
            {
                set_output_color(get_color("bright-blue"));
            }

            //If the location is flagged, simply place a flag character there.
            if(flagged)
            {
                if(!pc_pos)
                    set_output_color(get_color("bright-black"));
                print("\u2691");
                set_output_color(clr);
                continue;
            }

            //Is it a mine?
            if(mine_bitmap[x][y])
            {
                if(revealed && !pc_pos)
                    set_output_color(get_color("red"));
                print(revealed ? "\u2622" : "\u2588");
            }
            //If not, it's revealed.
            else
            {
                if(!revealed)
                {
                    print("\u2588");
                    set_output_color(clr);
                    continue;
                }

                //Get the mines and check if we should even print the number.
                int nearby_mines = get_nearby_mines(x, y);
                if(nearby_mines == 0)
                {
                    print("-");
                    set_output_color(clr);
                    continue;
                }

                char buf[10] = {0};
                itoa(nearby_mines, buf, 10);

                if(!pc_pos)
                {
                    if(nearby_mines == 1)
                        set_output_color(get_color("green"));
                    else if(nearby_mines == 2)
                        set_output_color(get_color("yellow"));
                    else if(nearby_mines > 2)
                        set_output_color(get_color("bright-magenta"));
                }
                print(buf);
            }
            set_output_color(clr);
        }
        print(VERT_WALL);
        print("\n");
    }
    print(BL_CORNER);
    for (int i = 0; i < MINE_WIDTH; ++i)
    {
        print(HORIZ_WALL);
    }
    print(BR_CORNER);
    print("\n");
    println("Use 'W' 'A' 'S' 'D' to move. Use 'F' to flag a mine.");
}

/**
 * Generates mines at random locations in the mine_bitmap.
 *
 * @param game_seed the game seed.
 */
void generate_mines(unsigned long long game_seed)
{
    unsigned long long previous_seed = get_seed();

    s_rand(game_seed);
    free_squares = MINE_WIDTH * MINE_HEIGHT;
    for (int x = 0; x < MINE_WIDTH; ++x)
    {
        for (int y = 0; y < MINE_HEIGHT; ++y)
        {
            //Generate random number for mine generation.
            double factor = next_random_lim(100) / 100.0;
            if(factor < MINE_GENERATION_FACTOR)
            {
                mine_bitmap[x][y] = true;
                mines++;
                free_squares--;
            }
        }
    }

    s_rand(previous_seed);
}

void start_minesweeper_game(unsigned long long game_seed)
{
    //Clear out memory for the maps.
    memset(mine_bitmap, 0, sizeof (mine_bitmap));
    memset(revealed_map, 0, sizeof (revealed_map));
    game_running = true;
    //Reset some flags.
    pc_x = pc_y = mines = mines_flagged = free_squares = revealed_squares = 0;
    generate_mines(game_seed);
    print_mine();
    while(game_running)
    {
        ms_game_tick();
        print_mine();
    }
    memset(revealed_map, 1, sizeof (revealed_map));
    pc_x = pc_y = -1;
    print_mine();

    if(mines == mines_flagged && free_squares == revealed_squares)
    {
        println("You win!");
    }
    else
    {
        println("You lose!");
    }

    game_running = false;
}
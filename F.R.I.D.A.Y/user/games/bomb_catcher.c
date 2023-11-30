//
// Created by Andrew Bowie on 1/27/23.
//

#include "bomb_catcher.h"
#include "stdio.h"
#include "stdbool.h"

///The width of the game screen
#define SCREEN_WIDTH 30
///The height of the game screen
#define SCREEN_HEIGHT 10

///Denotes the game being in an active state.
static bool game_active = false;
///The points the player has accumulated.
static int points = 0;
///The amount of 'time ticks' this game has used.
static int time = 0;

///The position of the catcher.
static int catcher_pos = 0;

///Stalls CPU time by spinning on a for loop.
void stall(void)
{
    for (int i = 0; i < 100000000; ++i);
}

///Resets the game to its initial state.
void reset(void)
{
    points = 0;
    time = 0;
    catcher_pos = SCREEN_WIDTH / 2;
}

///Draws the screen for the game.
void draw_scr(void)
{
    clearscr();
    for (int i = 0; i < SCREEN_HEIGHT; ++i)
    {
        print("|");
        for (int j = 0; j < SCREEN_WIDTH; ++j)
        {
            print(" ");
        }
        print("|\n");
    }

    //Print the catcher.
    for (int i = 0; i < catcher_pos; ++i)
    {
        print(" ");
    }
    print("\\-/");
    for (int i = 0; i < SCREEN_WIDTH - catcher_pos; ++i)
    {
        print(" ");
    }
}

///'ticks' the game.
void game_tick(void)
{
    time++;

    draw_scr();

    char read = pollc();
    printf("%c\n", read);
}

void start_bombcatcher(void)
{
    reset();
    game_active = true;

    while(game_active) {
        stall();
        game_tick();
    }
}

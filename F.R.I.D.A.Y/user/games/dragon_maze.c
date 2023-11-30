//
// Created by Andrew Bowie on 3/29/23.
//

#include "dragon_maze.h"
#include "stdbool.h"
#include "stdio.h"
#include "hash_map.h"
#include "linked_list.h"
#include "math.h"
#include "memory.h"
#include "mpx/heap.h"
#include "string.h"
#include "print_format.h"

#define EMPTY ' '
#define FOUR_WAY_WALL '+'
#define HORIZONTAL_WALL '-'
#define VERTICAL_WALL '|'
#define FINISH 'F'
#define PRINCESS 'P'
#define DRAGON 'D'
#define HERO_WITH_PRINCESS 'H'
#define HERO_NO_PRINCESS 'h'

///The length of sight for the dragon. Used in medium and up difficulties.
#define DRAGON_SIGHT_LENGTH 6

///The 'smell' length of the dragon. Used for path finding.
#define DRAGON_SMELL_LENGTH 9

///The default length of the maze.
#define MAZE_LENGTH 21
///The default height of the maze.
#define MAZE_HEIGHT 11

///An enum representing the difficulty of the running game.
typedef enum {
    ///In easy mode, there is no added challenge.
    EASY,
    ///In normal mode, the maze is initially obscured, but can be mapped out.
    NORMAL,
    ///In hard mode, the dragon is smarter and can track you down.
    HARD,
} difficulty_t;

///An enum representing direction.
typedef enum {
    ///Represents the 'up' direction.
    W,
    ///Represents the 'left' direction.
    A,
    ///Represents the 'down' direction.
    S,
    ///Represents the 'right' direction.
    D,
} direction_t;

///A struct for a 2d coordinate
typedef struct
{
    ///The y coordinate
    int x;
    ///The z coordinate
    int y;
} coordinate_t;

/**
 * Gets a direction enum from the given character.
 *
 * @param c the character being read.
 * @return the direction.
 */
direction_t direction_from_char(char c)
{
    switch (c)
    {
        case 'W':
        case 'w':
            return W;
        case 'A':
        case 'a':
            return A;
        case 'D':
        case 'd':
            return D;
        case 'S':
        case 's':
            return S;
        default:
            return -1;
    }
}

/**
 * @brief Gets the opposite of the given direction.
 *
 * @param direction the direction.
 * @return the opposite direction.
 */
direction_t get_opposite(direction_t direction)
{
    return ui_realmod((int) direction + 2, 4);
}

/**
 * Shifts the given coordinate by the given direction.
 *
 * @param coordinate the coordinate.
 * @param direction the direction.
 * @param multi the multiplier of shift.
 * @return the shifted coordinate.
 */
coordinate_t shift(coordinate_t coordinate, direction_t direction, int multi)
{
    coordinate.x += (direction == A ? -1 : direction == D ? 1 : 0) * multi;
    coordinate.y += (direction == W ? -1 : direction == S ? 1 : 0) * multi;
    return coordinate;
}

/**
 * Compares equality between the two coordinates.
 *
 * @param c1 the first coordinate.
 * @param c2 the second coordinate.
 * @return true if the coordinates are equal, false if not.
 */
bool coordinate_eq(coordinate_t *c1, coordinate_t *c2)
{
    return c1->x == c2->x && c1->y == c2->y;
}

/**
 * @brief Checks if the two coordinate are adjacent to each other.
 *
 * @param c1 the first coordinate.
 * @param c2 the second coordinate.
 * @return
 */
bool is_adjacent(coordinate_t c1, coordinate_t c2)
{
    int diffX = abs(c1.x - c2.x);
    int diffY = abs(c1.y - c2.y);

    return (diffX <= 1 && diffY == 0) || (diffX == 0 && diffY <= 1);
}

/**
 * @brief Checks if the given char represents a wall.
 * @param c the char.
 * @return if it's a wall.
 */
bool is_wall(char c)
{
    return c == FOUR_WAY_WALL || c == HORIZONTAL_WALL || c == VERTICAL_WALL;
}

/**
 * Hashes the given coordinate.
 *
 * @param coordinate the coordinate to hash.
 * @return the hash of the coordinate.
 */
int coordinate_hash(coordinate_t *coordinate)
{
    return (coordinate->x * 31 + coordinate->y * 31) * 31;
}

///The maze board struct.
typedef struct
{
    ///The pieces that make up the board.
    char board_pieces[MAZE_HEIGHT][MAZE_LENGTH];

    ///The location of the hero.
    coordinate_t hero_location;
    ///The dragon's location.
    coordinate_t dragon_location;
    ///The princess' location.
    coordinate_t princess_location;
} maze_board_t;

///If the game is currently running.
static bool running = false;
///Denotes if the dragon is still alive.
static bool dragon_alive = false;
///If the hero is currently holding the princess.
static bool holding_princess = false;
///The current hero symbol to use.
static char hero_symbol = HERO_NO_PRINCESS;
///The current difficulty in use.
static difficulty_t difficulty = EASY;
///The active maze board for the current run of the game.
static maze_board_t board;
///A list used to 'inform' the player of something happening.
static linked_list *inform_list;

///The map to use for visited tiles in maze generation. It's also used for the 'discovered' tiles in harder difficulties.
static bool visited_map[MAZE_HEIGHT][MAZE_LENGTH] = {{0}};

/**
 * @brief Sets the given piece at the given location.
 *
 * @param coordinate the coordinate.
 * @param c the piece.
 */
void set_piece(coordinate_t coordinate, char c)
{
    board.board_pieces[coordinate.y][coordinate.x] = c;
}

/**
 * @brief Checks if the given coordinate is on the board.
 * @param coordinate the coordinate.
 * @return true if the coordinate is on the board.
 */
bool is_on_board(coordinate_t coordinate)
{
    return coordinate.x >= 0 && coordinate.x < MAZE_LENGTH && coordinate.y >= 0 && coordinate.y <= MAZE_HEIGHT;
}

/**
 * @brief Gets the piece at the given location.
 *
 * @param coordinate the coordinate of the piece.
 * @return the piece value there.
 */
char get_piece(coordinate_t coordinate)
{
    return board.board_pieces[coordinate.y][coordinate.x];
}

/**
 * @brief This function takes the hero's current position and adds the 'renderable' positions from it.
 * Used for NORMAL+ difficulties.
 */
void add_renderable_positions(void)
{
    if(difficulty == EASY)
        return;

    for(direction_t direc = 0; direc <= D; direc++)
    {
        int index = 0;
        coordinate_t hero_location = board.hero_location;
        coordinate_t current = {0};

        //Iterate outward in a straight line.
        while(is_on_board(current = shift(hero_location, direc, index++)))
        {
            char at_loc = get_piece(current);
            if(is_wall(at_loc) || at_loc == FINISH)
            {
                visited_map[current.y][current.x] = true;

                //Spread out against the wall if necessary.
                if(at_loc == VERTICAL_WALL)
                {
                    coordinate_t wall_up = shift(current, W, 1);
                    coordinate_t wall_down = shift(current, S, 1);
                    visited_map[wall_up.y][wall_up.x] = true;
                    visited_map[wall_down.y][wall_down.x] = true;
                }
                else if(at_loc == HORIZONTAL_WALL)
                {
                    coordinate_t wall_left = shift(current, A, 1);
                    coordinate_t wall_right = shift(current, D, 1);
                    visited_map[wall_left.y][wall_left.x] = true;
                    visited_map[wall_right.y][wall_right.x] = true;
                }
                break;
            }

            //Add all the adjacent tiles.
            for(direction_t sub_dir = 0; sub_dir <= D; sub_dir++)
            {
                coordinate_t new_pos = shift(current, sub_dir, 1);
                visited_map[new_pos.y][new_pos.x] = true;
            }
        }
    }
}

/**
 * @brief The third, and final, step of board generation. Creates the paths through the maze.
 *
 * @param coordinate the coordinates.
 * @param end_points the end points list.
 */
void check_location(coordinate_t coordinate, linked_list *end_points)
{
    linked_list *queue = nl_unbounded();
    add_item(queue, (void *) W);
    add_item(queue, (void *) A);
    add_item(queue, (void *) S);
    add_item(queue, (void *) D);

    bool found = false;
    while(queue->_size > 0)
    {
        direction_t direc = (direction_t) remove_item_unsafe(queue, (int) next_random_lim(queue->_size));
        coordinate_t new_visit = shift(coordinate, direc, 2);

        if(new_visit.x < 0 || new_visit.x >= MAZE_LENGTH ||
                new_visit.y < 0 || new_visit.y >= MAZE_HEIGHT || visited_map[new_visit.y][new_visit.x])
            continue;

        //Check if the connection location is on the edge.
        coordinate_t connection_loc = shift(coordinate, direc, 1);
        if(connection_loc.x == 0 || connection_loc.y == 0 || connection_loc.x + 1 == MAZE_LENGTH || connection_loc.y + 1 == MAZE_HEIGHT)
            continue;

        //Get the piece.
        char piece = board.board_pieces[connection_loc.y][connection_loc.x];
        if(piece == EMPTY)
            continue;

        found = true;
        visited_map[new_visit.y][new_visit.x] = true;
        board.board_pieces[connection_loc.y][connection_loc.x] = EMPTY;

        //Recursively continue.
        check_location(new_visit, end_points);
    }

    if(!found)
    {
        coordinate_t *alloc_coord = sys_alloc_mem(sizeof(coordinate_t));
        alloc_coord->x = coordinate.x;
        alloc_coord->y = coordinate.y;
        add_item(end_points, alloc_coord);
    }

    ll_clear_free(queue, false);
    sys_free_mem(queue);
}

/**
 * @brief The second step of board generation. Controls the depth first generation of the paths
 * and places the hero, dragon, and princess when complete.
 */
void fill_randomly(void)
{
    linked_list *list = nl_unbounded();

    coordinate_t *origin = sys_alloc_mem(sizeof (coordinate_t));
    origin->x = origin->y = 1;

    while(list->_size < 3)
    {
        memset(visited_map, 0, sizeof(visited_map));
        visited_map[origin->y][origin->x] = true;
        ll_clear_free(list, true);

        check_location(*origin, list);
    }

    sys_free_mem(origin);

    //Get all the points for hero, dragon, and princess.
    coordinate_t *hero_point = remove_item_unsafe(list, (int) next_random_lim(list->_size));
    coordinate_t *dragon_point = remove_item_unsafe(list, (int) next_random_lim(list->_size));
    coordinate_t *princess_point = remove_item_unsafe(list, (int) next_random_lim(list->_size));

    set_piece(*hero_point, HERO_NO_PRINCESS);
    set_piece(*dragon_point, DRAGON);
    set_piece(*princess_point, PRINCESS);

    //Set the points for all the characters.
    board.hero_location = *hero_point;
    board.dragon_location = *dragon_point;
    board.princess_location = *princess_point;

    sys_free_mem(hero_point);
    sys_free_mem(dragon_point);
    sys_free_mem(princess_point);
    ll_clear_free(list, true);

    bool found = false;
    coordinate_t finish_line;

    //Find a valid point to put the finish line.
    while(!found)
    {
        bool cardinal_direc = next_rand_bool();
        bool positive_direc = next_rand_bool();

        int coordinate = (int) (cardinal_direc ? next_random_lim(MAZE_LENGTH - 2) : next_random_lim(MAZE_HEIGHT - 2));

        //Get the coordinate and shift it.
        finish_line = (coordinate_t) {
                .x = cardinal_direc ? coordinate + 1 : (positive_direc ? MAZE_LENGTH - 1 : 0),
                .y = cardinal_direc ? (positive_direc ? MAZE_HEIGHT - 1 : 0) : coordinate + 1
        };
        direction_t direc = cardinal_direc ? (positive_direc ? W : S) : (positive_direc ? A : D);

        coordinate_t shifted = shift(finish_line, direc, 1);

        if(board.board_pieces[shifted.y][shifted.x] == EMPTY)
        {
            found = true;
        }
    }

    //Set the finish spot.
    board.board_pieces[finish_line.y][finish_line.x] = FINISH;

    //Do some cleanup.
    ll_clear_free(list, true);
    sys_free_mem(list);
}

/**
 * @brief Prints the current game board.
 */
void print_board(void)
{
    clearscr();

    if(difficulty == HARD)
        memset(visited_map, false, sizeof(visited_map));

    add_renderable_positions();
    for (int y = 0; y < MAZE_HEIGHT; ++y)
    {
        //Create a copy of the string and print it.
        char string[MAZE_LENGTH + 1] = {0};
        for (int x = 0; x < MAZE_LENGTH; ++x)
        {
            string[x] = (char) (visited_map[y][x] ? board.board_pieces[y][x] : '#');
        }

        println(string);
    }

    //Print all items from the inform list.
    while(inform_list->_size > 0)
    {
        char *item = remove_item_unsafe(inform_list, 0);
        println(item);
    }
}

/**
 * @brief The first step of board generations. Fills the board with generic walls.
 */
void generate_board(void)
{
    //Insert barriers for all locations.
    for (int x = 0; x < MAZE_LENGTH; ++x)
    {
        for (int y = 0; y < MAZE_HEIGHT; ++y)
        {
            if (y % 2 == 0)
            {
                if (x % 2 == 0)
                    board.board_pieces[y][x] = FOUR_WAY_WALL;
                else
                    board.board_pieces[y][x] = HORIZONTAL_WALL;
            }
            else
            {
                if (x % 2 == 0)
                    board.board_pieces[y][x] = VERTICAL_WALL;
                else
                    board.board_pieces[y][x] = EMPTY;
            }
        }
    }

    fill_randomly();
}

/**
 * @brief This function, called once per tick loop, controls hero movement.
 */
void move_hero(void)
{
    if(holding_princess)
    {
        println("Princess: Held");
    }
    else
    {
        printf("Princess: %d, %d\n", board.princess_location.x, board.princess_location.y);
    }

    if(!dragon_alive)
    {
        println("Dragon: Defeated");
    }
    else
    {
        printf("Dragon: %d, %d\n", board.dragon_location.x, board.dragon_location.y);
    }

    println("Please enter a direction to move. (W, A, S, D) (Press 'F' to do nothing)");

    char next_char = getc();

    int direc = -1;
    while((direc = direction_from_char(next_char)) == -1)
    {
        if(next_char == 'F' || next_char == 'f')
            return;

        println("Please enter a valid direction! (W, A, S, D, F)");
        next_char = getc();
    }

    //Check the coordinate's character.
    direction_t direction = (direction_t) direc;
    coordinate_t shifted = shift(board.hero_location, direction, 1);
    char moving_to = board.board_pieces[shifted.y][shifted.x];

    if(is_wall(moving_to))
    {
        add_item(inform_list, "D'oh!");
        return;
    }

    //Check if the player is moving to the finish.
    if(moving_to == FINISH)
    {
        if(!dragon_alive && !holding_princess)
        {
            add_item(inform_list, "I've already killed the dragon, I need to save the princess!");
            return;
        }

        set_piece(board.hero_location, EMPTY);
        board.hero_location = shifted;
        set_piece(board.hero_location, hero_symbol);

        if(holding_princess)
        {
            if(!dragon_alive)
            {
                add_item(inform_list, "You escaped with the princess and slew the dragon! You're a true hero!");
            }
            else
            {
                add_item(inform_list, "You escaped with the princess, but the dragon is still alive! You lived to fight another day...");
            }
        }
        else
        {
            add_item(inform_list, "You escaped with your life, but the dragon lives and the princess remains in the castle! Can you even call yourself a hero?");
        }
        running = false;
        return;
    }

    if(moving_to == PRINCESS)
    {
        holding_princess = true;
        hero_symbol = HERO_WITH_PRINCESS;
    }

    //Update the hero's location.
    set_piece(board.hero_location, EMPTY);
    board.hero_location = shifted;
    set_piece(board.hero_location, hero_symbol);
}

/**
 * Finds a direction heading for the dragon to move in. Methods depend on difficulty.
 *
 * @return the dragon's movement heading.
 */
direction_t find_dragon_movement(void)
{
    //In hard mode, the dragon can path-find to the hero.
    if(difficulty >= HARD)
    {
        struct breadth_first_node {
            //The coordinate origin.
            coordinate_t coordinate;
            //The direction pointing to the previous tile.
            direction_t direction;
            //The initial offset used.
            direction_t initial_offset;
            //The amount of steps we've taken in this direction.
            int steps;
        };
        linked_list *breadth_first_queue = nl_unbounded();

        struct breadth_first_node *origin_node = sys_alloc_mem(sizeof (struct breadth_first_node));
        memset(origin_node, 0, sizeof (*origin_node));
        origin_node->coordinate = board.dragon_location;
        origin_node->steps = 0;
        origin_node->direction = -1; //This will allow us to iterate in every direction.
        origin_node->initial_offset = -1;
        add_item(breadth_first_queue, origin_node);

        //Iterate while the queue's size is > 0.
        while(breadth_first_queue->_size > 0)
        {
            struct breadth_first_node *node = remove_item_unsafe(breadth_first_queue, 0);
            for(direction_t direction = W; direction <= D; direction++)
            {
                if(direction == node->direction)
                    continue;

                //Shift the coordinate.
                coordinate_t shifted = shift(node->coordinate, direction, 1);
                char piece = get_piece(shifted);
                if(piece != EMPTY)
                {
                    //Check if we've found the hero.
                    if(coordinate_eq(&board.hero_location, &shifted))
                    {
                        int offset = node->initial_offset;
                        sys_free_mem(node);
                        destroy_list(breadth_first_queue, true);
                        if(offset == -1)
                            return direction;

                        return offset;
                    }
                    continue;
                }

                if(node->steps + 1 > DRAGON_SMELL_LENGTH)
                {
                    continue;
                }

                //Create a new node and add it.
                struct breadth_first_node *next_node = sys_alloc_mem(sizeof (struct breadth_first_node));
                memset(next_node, 0, sizeof (*next_node));
                if((int) node->initial_offset == -1)
                    next_node->initial_offset = direction;
                else
                    next_node->initial_offset = node->initial_offset;
                next_node->direction = get_opposite(direction);
                next_node->coordinate = shifted;
                next_node->steps = node->steps + 1;
                add_item(breadth_first_queue, next_node);
            }

            sys_free_mem(node);
        }

        destroy_list(breadth_first_queue, true);
    }

    //In normal mode, the dragon can see the hero in straight lines.
    if(difficulty >= NORMAL)
    {
        //Check if we can find the hero by sight,
        for (direction_t direction = 0; direction <= D; ++direction)
        {
            for (int i = 1; i < DRAGON_SIGHT_LENGTH; ++i)
            {
                coordinate_t shifted = shift(board.dragon_location, direction, i);

                if(coordinate_eq(&shifted, &board.hero_location))
                    return direction;

                //Otherwise, check if we've hit a wall.
                if(is_wall(get_piece(shifted)))
                    break;
            }
        }
    }

    return (direction_t) next_random_lim(4);
}

/**
 * @brief Controls dragon movement, called once per tick loop.
 */
void move_dragon(void)
{
    direction_t movement_direc = find_dragon_movement();
    coordinate_t new_coordinate = shift(board.dragon_location, movement_direc, 1);

    char piece_at = board.board_pieces[new_coordinate.y][new_coordinate.x];

    //Check if the dragon is adjacent to the hero.
    if(is_adjacent(board.dragon_location, board.hero_location))
    {
        //TODO Implement dragon fighting. May need to wait until time checking works.
        running = false;
        add_item(inform_list, "You died, game over!");
        return;
    }

    if(is_wall(piece_at) || piece_at == FINISH)
        return;

    set_piece(board.dragon_location, EMPTY);
    board.dragon_location = new_coordinate;
    set_piece(board.dragon_location, DRAGON);

    //Check if the dragon is adjacent to the hero.
    if(is_adjacent(board.dragon_location, board.hero_location))
    {
        //TODO Implement dragon fighting.
        running = false;
        add_item(inform_list, "You died, game over!");
    }
}

void start_dragonmaze_game(void)
{
    //Initialize all the values.
    memset(&board, 0, sizeof(maze_board_t));
    dragon_alive = true;
    holding_princess = false;
    hero_symbol = HERO_NO_PRINCESS;
    inform_list = nl_unbounded();

    //Ask the user for a difficulty.
    int diff_int = -1;
    while(diff_int == -1)
    {
        println("Which difficulty would you like to play? Easy, Normal, or Hard?");
        char input[11] = {0};
        gets(input, 10);

        //Check if the string is valid for any difficulty.
        if(strcicmp(input, "easy") == 0)
            diff_int = EASY;
        else if(strcicmp(input, "normal") == 0)
            diff_int = NORMAL;
        else if(strcicmp(input, "hard") == 0)
            diff_int = HARD;
    }

    difficulty = (difficulty_t) diff_int;
    generate_board();

    //In easy mode, we don't need to worry about hiding tiles.
    memset(visited_map, difficulty == EASY, sizeof(visited_map));

    print_board();

    //Begin the game loop.
    running = true;
    while(running)
    {
        move_hero();

        if(dragon_alive)
            move_dragon();

        print_board();
    }

    //Do a final cleanup.
    ll_clear(inform_list);
    sys_free_mem(inform_list);
}
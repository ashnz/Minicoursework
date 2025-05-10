#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define SPACE_SIZE 18
#define JUNK_CNT 50
#define TOP_PLAYERS 5

typedef struct {
    int x, y;
} Position;

typedef struct {
    Position pos;
    int fuel;
    int junk_collected;
    int moves;
} Player;

typedef struct {
    Position pos;
} Asteroid;

typedef struct {
    Player player;
    Asteroid asteroid;
    Position junk[JUNK_CNT];
    int junk_count;
    int difficulty;
} GameState;

//function declerations
Position randomxy(const GameState *state);
void grid_design(const GameState *state);
void health_bar(const GameState *state);
void score_save(const char *filename, int moves);
void top_players(const char *filename);
GameState makeup_arena(int difficulty);
int select_difficulty();
void player_moves(GameState *state, char direction);
void asteroid_moves(GameState *state);
int check_destruct(const GameState *state);
void original_loop(GameState *state);

#endif /* GAME_H */
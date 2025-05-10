#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define GRID_SIZE 18
#define MAX_JUNK 50
#define MAX_SCORES 5

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
    Position junk[MAX_JUNK];
    int junk_count;
    int difficulty;
} GameState;

// ===== UTILS =====
Position random_position(const GameState *state) {
    Position pos;
    int conflict;
    do {
        conflict = 0;
        pos.x = rand() % GRID_SIZE;
        pos.y = rand() % GRID_SIZE;
        if (pos.x == state->player.pos.x && pos.y == state->player.pos.y)
            conflict = 1;
        if (pos.x == state->asteroid.pos.x && pos.y == state->asteroid.pos.y)
            conflict = 1;
    } while (conflict);
    return pos;
}

// ===== DISPLAY =====
void display_grid(const GameState *state) {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (x == state->player.pos.x && y == state->player.pos.y) printf("P ");
            else if (x == state->asteroid.pos.x && y == state->asteroid.pos.y) printf("A ");
            else {
                int junk_here = 0;
                for (int i = 0; i < state->junk_count; i++) {
                    if (state->junk[i].x == x && state->junk[i].y == y) {
                        junk_here = 1;
                        break;
                    }
                }
                printf(junk_here ? "J " : ". ");
            }
        }
        printf("\n");
    }
}

void display_status(const GameState *state) {
    printf("Fuel: %d | Junk Collected: %d | Moves: %d\n",
           state->player.fuel, state->player.junk_collected, state->player.moves);
}

// ===== FILE I/O =====
void load_intro(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            printf("%s", line);
        }
        fclose(f);
    }
}

void save_score(const char *filename, int moves) {
    int scores[MAX_SCORES];
    int count = 0;
    FILE *f = fopen(filename, "r");
    if (f) {
        while (fscanf(f, "%d", &scores[count]) == 1 && count < MAX_SCORES)
            count++;
        fclose(f);
    }

    scores[count++] = moves;

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (scores[j] < scores[i]) {
                int temp = scores[i];
                scores[i] = scores[j];
                scores[j] = temp;
            }
        }
    }

    f = fopen(filename, "w");
    for (int i = 0; i < count && i < MAX_SCORES; i++) {
        fprintf(f, "%d\n", scores[i]);
    }
    fclose(f);
}

void show_leaderboard(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;
    printf("Top Scores:\n");
    int moves, rank = 1;
    while (fscanf(f, "%d", &moves) == 1) {
        printf("%d. %d moves\n", rank++, moves);
    }
    fclose(f);
}

// ===== GAME LOGIC =====
GameState init_game(int difficulty) {
    GameState state;
    state.player.pos.x = 0;
    state.player.pos.y = 0;
    state.player.fuel = 100 - (difficulty * 10);
    state.player.junk_collected = 0;
    state.player.moves = 0;
    state.asteroid.pos.x = GRID_SIZE - 1;
    state.asteroid.pos.y = GRID_SIZE - 1;
    state.junk_count = 10 + (difficulty * 5);
    state.difficulty = difficulty;

    for (int i = 0; i < state.junk_count; i++) {
        state.junk[i] = random_position(&state);
    }

    return state;
}

int select_difficulty() {
    int level;
    printf("Select difficulty (1 = Easy, 2 = Medium, 3 = Hard): ");
    scanf("%d", &level);
    if (level < 1 || level > 3) level = 1;
    return level;
}

void move_player(GameState *state, char direction) {
    int x = state->player.pos.x;
    int y = state->player.pos.y;

    if (direction == 'w' && y > 0) y--;
    else if (direction == 's' && y < GRID_SIZE - 1) y++;
    else if (direction == 'a' && x > 0) x--;
    else if (direction == 'd' && x < GRID_SIZE - 1) x++;

    state->player.pos.x = x;
    state->player.pos.y = y;

    for (int i = 0; i < state->junk_count; i++) {
        if (state->junk[i].x == x && state->junk[i].y == y) {
            state->player.junk_collected++;
            state->player.fuel += 10;
            state->junk[i].x = -1;
            state->junk[i].y = -1;
            break;
        }
    }
}

void move_asteroid(GameState *state) {
    if (state->asteroid.pos.x > 0) state->asteroid.pos.x--;
    if (state->asteroid.pos.y > 0) state->asteroid.pos.y--;
}

int check_collision(const GameState *state) {
    return (state->player.pos.x == state->asteroid.pos.x &&
            state->player.pos.y == state->asteroid.pos.y);
}

void game_loop(GameState *state) {
    char input;
    while (1) {
        display_grid(state);
        display_status(state);

        printf("Move (WASD), Q to quit: ");
        scanf(" %c", &input);
        if (input == 'q' || input == 'Q') {
            printf("Game Over. You quit.\n");
            break;
        }

        move_player(state, input);
        move_asteroid(state);
        state->player.moves++;
        state->player.fuel--;

        if (check_collision(state)) {
            printf("Crashed into asteroid. Game Over.\n");
            break;
        }

        if (state->player.fuel <= 0) {
            printf("You ran out of fuel. Game Over.\n");
            break;
        }

        if (state->player.junk_collected >= 10 + state->difficulty * 5) {
            printf("Congratulations! You cleaned up space in %d moves!\n", state->player.moves);
            save_score("leaderboard.txt", state->player.moves);
            show_leaderboard("leaderboard.txt");
            break;
        }
    }
}

// ===== MAIN =====
int main() {
    srand(time(NULL));
    load_intro("levels.txt");
    int difficulty = select_difficulty();
    GameState state = init_game(difficulty);
    game_loop(&state);
    return 0;
}

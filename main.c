#include "game.h"//including game header file

//creating random positions
Position randomxy(const GameState *state) {
    Position pos;
    int disregard;
    do {
        disregard= 0;
        pos.x = rand() % SPACE_SIZE;//generating random numbers upto 18
        pos.y = rand() % SPACE_SIZE;
        if (pos.x == state->player.pos.x && pos.y == state->player.pos.y)//check whether the player is already there
            disregard = 1;
        if (pos.x == state->asteroid.pos.x && pos.y == state->asteroid.pos.y)//check whether the asteroid is there
            disregard = 1;
    } while (disregard);//loops till there's a conflict
    return pos;//returns the random generated positions
}

//function for the design and entity placement
void grid_design(const GameState *state) {
    for (int y = 0; y < SPACE_SIZE; y++) {//row check
        for (int x = 0; x < SPACE_SIZE; x++) {//x in eac row check
            if (x == state->player.pos.x && y == state->player.pos.y) printf("P ");//player symbol
            else if (x == state->asteroid.pos.x && y == state->asteroid.pos.y) printf("A ");//asteroid symbol
            else {
                int junk_here = 0;
                for (int i = 0; i < state->junk_count; i++) {
                    if (state->junk[i].x == x && state->junk[i].y == y) {
                        junk_here = 1;
                        break;
                    }
                }
                printf(junk_here ? "J " : ". ");//if statement in short form and symbol for junk and available spots
            }
        }
        printf("\n");
    }
}
//function for health bar
void health_bar(const GameState *state) {
    printf("Fuel - %d /---/ Junk Recycled - %d /---/ Move count - %d\n",
           state->player.fuel, state->player.junk_collected, state->player.moves);//prints the updates values(done by pointers in further functions
}

//function for saving score
void score_save(const char *filename, int moves) {
    int scores[TOP_PLAYERS];
    int cnt = 0;
    FILE *f = fopen(filename, "r");//file open in read only mode
    if (f) {
        while (fscanf(f, "%d", &scores[cnt]) == 1 && cnt < TOP_PLAYERS)
            cnt++;//saving the scores
        fclose(f);
    }

    scores[cnt++] = moves;//updating new score
//organizing the lowest count to top and highest count to bottom
    for (int i = 0; i < cnt - 1; i++) {
        for (int j = i + 1; j < cnt; j++) {
            if (scores[j] < scores[i]) {
                int temp = scores[i];
                scores[i] = scores[j];
                scores[j] = temp;
            }
        }
    }
// file open with write mode
    f = fopen(filename, "w");
    for (int i = 0; i < cnt && i < TOP_PLAYERS; i++) {
        fprintf(f, "%d\n", scores[i]);
    }//filtering the top 5
    fclose(f);
}
//printing the top 5 from file
void top_players(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;//checking the file presence
    printf("Top Scores:\n");
    int moves, rank = 1;
    while (fscanf(f, "%d", &moves) == 1) {
        printf("%d. %d moves\n", rank++, moves);//moves are automatically being incremented
    }
    fclose(f);
}

//function for setting up the locations of each entity
GameState makeup_arena(int difficulty) {
    GameState state;
    state.player.pos.x = 0;//setting top player at top most
    state.player.pos.y = 0;
    state.difficulty = difficulty;
    state.player.fuel = 100 - (difficulty * 10);//initial fuel level
    state.player.junk_collected = 0;
    state.player.moves = 0;
    state.asteroid.pos = randomxy(&state);
    state.junk_count = 10 + (difficulty * 5);//setting junk count based on difficulty
    for (int i = 0; i < state.junk_count; i++) {
        state.junk[i] = randomxy(&state);//assigning random junk spots
    }

    return state;//returns a gamestate struct
}
//function to assign difficulty on input
int select_difficulty() {
    int level;
    printf("Enter difficulty - Easy(1) , Medium(2) , Hard(3) - ");
    scanf("%d", &level);
    if (level < 1 || level > 3) {
        level = 1;//defaul easy level for invalid inputs
    }
    return level;
}
//function for player movements
void player_moves(GameState *state, char direction) {
    int x = state->player.pos.x;//player's current location
    int y = state->player.pos.y;
//control the input directions by x y coordinates (increments and decrements)
    if (direction == 'w' && y > 0) {
        y--;
    }
    else if (direction == 's' && y < SPACE_SIZE - 1) {
        y++;
    }
    else if (direction == 'a' && x > 0) {
        x--;
    }
    else if (direction == 'd' && x < SPACE_SIZE - 1) {
        x++;
    }

    state->player.pos.x = x;//updating position
    state->player.pos.y = y;

    for (int i = 0; i < state->junk_count; i++) {
        if (state->junk[i].x == x && state->junk[i].y == y) {
            state->player.junk_collected++;
            state->player.fuel += 10;//updating fuel once junk collected
            state->junk[i].x = -1;
            state->junk[i].y = -1;// removal of junk
            break;
        }
    }
}
//function for the movement of asteroid
void asteroid_moves(GameState *state) {
    if (state->asteroid.pos.x > 0) state->asteroid.pos.x--;
    if (state->asteroid.pos.y > 0) state->asteroid.pos.y--;
}
//checking the collision
int check_destruct(const GameState *state) {
    return (state->player.pos.x == state->asteroid.pos.x &&
            state->player.pos.y == state->asteroid.pos.y);
}
//function for the game's activity loop
void original_loop(GameState *state) {
    char input;//for WASD
    int points = 0;

    while (1) {
       grid_design(state);//prints the grid
        health_bar(state);//prints healthbar

        printf("Up(W) Down(S) Left(A) Right(D) Exit(Q): ");
        scanf(" %c", &input);
   //if the player decides to quit
        if (input == 'q' || input == 'Q') {
            printf("Game Over. You quit.\n");
            break;
        }

        player_moves(state, input);//player's moving
        asteroid_moves(state);//asteroid's moving
        state->player.moves++;//incrementing the moves
        //reduction of fuel based on difficulty
        if (state->difficulty == 1) {
            points -= 2;
            state->player.fuel -= 2;
        } else if (state->difficulty == 2) {
            points -= 3;
            state->player.fuel -= 3;
        } else if (state->difficulty == 3) {
            points -= 5;
            state->player.fuel -= 5;
        }
        //increment of fuel as a reward based on difficulty
        for (int i = 0; i < state->junk_count; i++) {
            if (state->junk[i].x == state->player.pos.x && state->junk[i].y == state->player.pos.y) {
                state->player.junk_collected++;

                if (state->difficulty == 1) {
                    points += 7;
                } else if (state->difficulty == 2) {
                    points += 10;
                } else if (state->difficulty == 3) {
                    points += 15;
                }
                state->junk[i].x = -1; //removes junk
                state->junk[i].y = -1;
                break;
            }
        }
        //game end once crashed
        if (check_destruct(state)) {
            printf("Crashed into asteroid. Game Over.\n");
            break;
        }
        //dame ends once out of fuel
        if (state->player.fuel <= 0) {
            printf("You ran out of fuel. Game Over.\n");
            break;
        }
        //winning message
        if (state->player.junk_collected >= 10 + state->difficulty * 5) {
            printf("Congratulations! You won by %d moves by earning %d points!\n", state->player.moves, points);
            score_save("leaderboard.txt", points);  //updating leader board
            top_players("leaderboard.txt");
            break;
        }
    }
}

int main() {
    srand(time(NULL));//to make random values at each round
    printf("Welcome to SpaceXplorer! Select the difficulty and follow the instructions. Happy exploring!\n");
    int difficulty = select_difficulty();
    GameState state = makeup_arena(difficulty);//running the game
    original_loop(&state);//passing pointer to original loop
    return 0;
}
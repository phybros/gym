#include "raylib.h"
#include "raymath.h"
#include <string.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define FIELD_WIDTH 80
#define FIELD_HEIGHT 80

int grid[FIELD_WIDTH][FIELD_HEIGHT];

int paused = 1;
int generation = 0;

const Color life_color = {
    .r = 34,
    .g = 144,
    .b = 200,
    .a = 255,
};

const Color grid_color = {
    .r = 50,
    .g = 50,
    .b = 50,
    .a = 255,
};

// make passing grid coords around easier
typedef struct {
    int x;
    int y;
} Vector2i;

Vector2i screen2grid(Vector2 grid_pos) {
    Vector2 p = GetMousePosition();
    p = Vector2Scale(p, (float)FIELD_WIDTH / (float)SCREEN_WIDTH);
    return (Vector2i) {
        .x = (int)floorf(p.x),
        .y = (int)floorf(p.y),
    };
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "gol");
    SetTargetFPS(60);

    char generation_label[32];

    while (!WindowShouldClose())
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2i g = screen2grid(GetMousePosition());   
            grid[g.x][g.y] = 1;
        }
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            Vector2i g = screen2grid(GetMousePosition());   
            grid[g.x][g.y] = 0;
        }

        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
        }

        int new_grid[FIELD_WIDTH][FIELD_HEIGHT];

        if (!paused) {
            for (int x = 0; x < FIELD_WIDTH; x++) {
                for (int y = 0; y < FIELD_HEIGHT; y++) {

                    // from https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
                    // Any live cell with fewer than two live neighbours dies, as if by underpopulation.
                    // Any live cell with two or three live neighbours lives on to the next generation.
                    // Any live cell with more than three live neighbours dies, as if by overpopulation.
                    // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
                    int num_neigh = 0;

                    for (int i = -1; i <= 1; i++) {
                        for (int j = -1; j <= 1; j++) {
                            if (i == 0 && j == 0) continue;
                            if (x + i < 0 || x + i > FIELD_WIDTH) continue;
                            if (y + j < 0 || y + j > FIELD_HEIGHT) continue;

                            if (grid[x+i][y+j]) {
                                num_neigh++;
                            }
                        }
                    }

                    if (grid[x][y]) {
                        if (num_neigh < 2 || num_neigh > 3) {
                            new_grid[x][y] = 0;
                        } else if (num_neigh == 2 || num_neigh == 3) {
                            new_grid[x][y] = 1;
                        }
                    } else {
                        if (num_neigh == 3) {
                            new_grid[x][y] = 1;
                        } else {
                            new_grid[x][y] = 0;
                        }
                    }

                }
            }

            memcpy(grid, new_grid, sizeof(new_grid));
            generation++;
        }
        BeginDrawing();
        ClearBackground(BLACK);

        for (int x = 0; x < FIELD_WIDTH; x++) {
            DrawLine(x * SCREEN_WIDTH / FIELD_WIDTH, 0, x * SCREEN_WIDTH / FIELD_WIDTH, SCREEN_HEIGHT, grid_color);

            for (int y = 0; y < FIELD_HEIGHT; y++) {

                if (grid[x][y]) {
                    DrawRectangle(x * SCREEN_WIDTH / FIELD_WIDTH, y * SCREEN_HEIGHT / FIELD_HEIGHT, SCREEN_HEIGHT / FIELD_HEIGHT, SCREEN_HEIGHT / FIELD_HEIGHT, life_color);
                }

                DrawLine(0, y * SCREEN_HEIGHT / FIELD_HEIGHT, SCREEN_WIDTH, y * SCREEN_HEIGHT / FIELD_HEIGHT, grid_color);
            }
        }

        if (paused) {
            DrawText("[ PAUSED ]", 2, 2, 20, YELLOW);
        } else {
            sprintf(generation_label, "GEN: %d", generation);
            DrawText(generation_label, 2, 2, 20, YELLOW);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

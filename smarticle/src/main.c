#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

const int screenWidth = 800;
const int screenHeight = 800;

// how many total particles
#define NUM_PARTICLES 1200
// how many different species
#define NUM_SPECIES 6

float max_range = 0.1;
float timestep = 0.02;
float force_scale_factor = 8.0;

Color species_colors[NUM_SPECIES];

// try using a "structure of arrays"
typedef struct {
    Vector2 positions[NUM_PARTICLES];
    Vector2 velocities[NUM_PARTICLES];
    int colors[NUM_PARTICLES];
} Particles;

Particles p;

typedef struct {
    float attraction;
} forces;

float attractions[NUM_SPECIES][NUM_SPECIES];

float randf_range(float min, float max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * (max - min);
}

int randi_range(int min, int max) {
    return (rand() % (max - min)) + min;
}

void initialize_forces_matrix() {
    for (int i = 0; i < NUM_SPECIES; i++) {
        for (int j = 0; j < NUM_SPECIES; j++) {
            attractions[i][j] = randf_range(-1, 1);
        }
    }
}


float force(float r, float attraction) {
    float beta = 0.2;

    if (r < beta) {
        return r / beta - 1;
    } else if (beta < r && r < 1) {
        return attraction * (1 - fabs(2 * r - 1 - beta) / (1 - beta));
    } else {
        return 0.0;
    }
}

void update_simulation_naive(float delta) {
    float friction = pow(0.5, timestep / 0.04);

    // update acceleration based on attraction forces
    // update velocity based on acceleration
    // reduce movement by friction else particles will fly away forever
    // update particle positions based on velocities

    for (int i = 0; i < NUM_PARTICLES; i++) {
        // always start from 0
        Vector2 acceleration = {0};

        for (int j = 0; j < NUM_PARTICLES; j++) {
            if (i == j) continue;

            Vector2 dirToOther = Vector2Normalize(Vector2Subtract(p.positions[j], p.positions[i]));
            float r = Vector2Distance(p.positions[i], p.positions[j]);

            float attraction = attractions[p.colors[i]][p.colors[j]];

            if (r > 0 && r <= max_range) {
                float f = force(r / max_range, attraction);
                acceleration.x += dirToOther.x * f;
                acceleration.y += dirToOther.y * f;
            }
        }

        acceleration = Vector2Scale(acceleration, max_range * force_scale_factor);

        p.velocities[i] = Vector2Scale(p.velocities[i], friction);
        p.velocities[i] = Vector2Add(p.velocities[i], Vector2Scale(acceleration, timestep));
    }


    for (int i = 0; i < NUM_PARTICLES; i++) {
        // update positions based on velocity
        p.positions[i].x += p.velocities[i].x * timestep;
        p.positions[i].y += p.velocities[i].y * timestep;
    }
}

void render() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        DrawCircle(p.positions[i].x * screenWidth, p.positions[i].y * screenHeight, 2, species_colors[p.colors[i]]);


    }
}

int main(int argc, char ** argv)
{
    const int PARTICLES_PER_SPECIES = (int)floor(NUM_PARTICLES / NUM_SPECIES);

    // seed the rng
    srand(time(NULL));

    // chill the logs out a bit
    SetTraceLogLevel(LOG_WARNING);

    // if we passed in a seed, use that for srand instead
    if (argc > 1) {
        unsigned int seed = 0;
        for (size_t c = 0; c < strlen(argv[1]); c++) {
            // just add all the characters in the first arg togehther :D 
            seed += (int)argv[1][c];
        }

        srand(seed);
    }

    // set the colors to random values
    for (int i = 0; i < NUM_SPECIES; i++) {
        species_colors[i] = (Color) {
            .r = randi_range(5, 255),
            .g = randi_range(5, 255),
            .b = randi_range(5, 255),
            .a = 255,
        };
    }


    // startup raylib
    InitWindow(screenWidth, screenHeight, "smarticle");
    SetTargetFPS(60);

    // initialize all the particles
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // random positions
        p.positions[i] = (Vector2){
            .x = randf_range(0, 1),
            .y = randf_range(0, 1),
        };

        p.colors[i] = (int)floor(i / PARTICLES_PER_SPECIES);
    }

    initialize_forces_matrix();

    // main looop
    while (!WindowShouldClose())
    {
        // press enter to change all the behaviours at once
        if (IsKeyPressed(KEY_ENTER)) {
            initialize_forces_matrix();
        }

        update_simulation_naive(GetFrameTime());

        // redner the particles!
        BeginDrawing();
            ClearBackground(BLACK);
            render();
            DrawFPS(2, 2);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

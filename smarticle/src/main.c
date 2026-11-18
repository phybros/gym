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

#define GRID_SIZE 10

int DEBUG = 0;
float max_range = 0.1;
float timestep = 0.02;
float force_scale_factor = 8.0;

Color species_colors[NUM_SPECIES];

// try using a "structure of arrays"
typedef struct {
    Vector2 positions[NUM_PARTICLES];
    Vector2 velocities[NUM_PARTICLES];
    int colors[NUM_PARTICLES];
    int grid_cell_x[NUM_PARTICLES];
    int grid_cell_y[NUM_PARTICLES];
} Particles;

Particles p;

typedef struct {
    float attraction;
} forces;

float attractions[NUM_SPECIES][NUM_SPECIES];
int nearby_particles[NUM_PARTICLES]; // pretty sure i can just reuse/overwrite this for each particle
int grid[GRID_SIZE][GRID_SIZE][NUM_PARTICLES];

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

void draw_grid() {
    if (!DEBUG) return;

    char label[6];

    float grid_factor = screenWidth / (float)GRID_SIZE;

    // this is so gross but works OK
    for (int x = 0; x < GRID_SIZE; x++) {
        if (x > 0) DrawLine(x * grid_factor, 0, x * grid_factor, screenHeight, GRAY);
        for (int y = 0; y < GRID_SIZE; y++) {
            if (y > 0) DrawLine(0, y * grid_factor, screenWidth, y * grid_factor, GRAY);

            sprintf(label, "%d,%d", x, y);        
            DrawText(label, x * grid_factor, y * grid_factor, 10, WHITE);
        }
    }
}
void update_grid_cells() {
    float grid_factor = screenWidth / (float)GRID_SIZE;

    for (int i = 0; i < NUM_PARTICLES; i++) {
        int scaledx = (int)(p.positions[i].x * screenWidth) / grid_factor;
        int scaledy = (int)(p.positions[i].y * screenHeight) / grid_factor;

        p.grid_cell_x[i] = scaledx;
        p.grid_cell_y[i] = scaledy;
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

int get_nearby_particles(int particle) {
    int num_nearby_particles = 0;

    // naive solution: just return all particles
    {
        for (int i = 0; i < NUM_PARTICLES; i++) {
            nearby_particles[num_nearby_particles] = i;
            num_nearby_particles++;
        }
    }

    // spatial grid?
    {
        // figure out which grid cell the particle is in
        // return all the particle ids for the 8 cells around it
        //
        // i need an array of all the cells which gets refreshed every timestep
        // for (int i = 0; i < NUM_PARTICLES; i++) {
        //
        //     for (int x = -1; x <= 1; x++) {
        //         for (int y = -1; y <= 1; y++) {
        //             if (p.grid_cell_x[i] == p.grid_cell_x[particle+x] && p.grid_cell_y[i] == p.grid_cell_y[particle+y]) {
        //                 nearby_particles[num_nearby_particles] = i;
        //                 num_nearby_particles++;
        //             }
        //         }
        //     }
        // }
    }

    return num_nearby_particles;
}

void update_simulation(float delta) {
    float friction = pow(0.5, delta / 0.04);

    // update acceleration based on attraction forces
    // update velocity based on acceleration
    // reduce movement by friction else particles will fly away forever
    // update particle positions based on velocities

    for (int i = 0; i < NUM_PARTICLES; i++) {
        // always start from 0
        Vector2 acceleration = {0};
        int n = get_nearby_particles(i);

        for (int j = 0; j < n; j++) {
            int o = nearby_particles[j];

            if (i == o) continue;

            Vector2 dirToOther = Vector2Normalize(Vector2Subtract(p.positions[o], p.positions[i]));
            float r = Vector2Distance(p.positions[i], p.positions[o]);

            float attraction = attractions[p.colors[i]][p.colors[o]];

            if (r > 0 && r <= max_range) {
                float f = force(r / max_range, attraction);
                acceleration.x += dirToOther.x * f;
                acceleration.y += dirToOther.y * f;
            }
        }

        acceleration = Vector2Scale(acceleration, max_range * force_scale_factor);

        p.velocities[i] = Vector2Scale(p.velocities[i], friction);
        p.velocities[i] = Vector2Add(p.velocities[i], Vector2Scale(acceleration, delta));
    }
    

    for (int i = 0; i < NUM_PARTICLES; i++) {
        // update positions based on velocity
        p.positions[i].x += p.velocities[i].x * delta;
        p.positions[i].y += p.velocities[i].y * delta;
    }

    update_grid_cells();
}

void render() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
       DrawCircle(p.positions[i].x * screenWidth, p.positions[i].y * screenHeight, 2, species_colors[p.colors[i]]);
    }

    draw_grid();
}

int main(int argc, char ** argv)
{
    const int PARTICLES_PER_SPECIES = (int)floor(NUM_PARTICLES / NUM_SPECIES);

    // seed the rng
    unsigned int seed = time(NULL);

    // chill the logs out a bit
    SetTraceLogLevel(LOG_WARNING);

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);

    // if we passed in a seed, use that for srand instead
    printf("Seed: ");
    if (argc > 1) {
        seed = 0;

        for (size_t c = 0; c < strlen(argv[1]); c++) {
            // just add all the characters in the first arg togehther :D 
            seed += (int)argv[1][c];
        }

        printf("%s\n", argv[1]);
    } else {
        printf("%d\n", seed);
    }

    srand(seed);

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

    update_grid_cells();

    // main looop
    while (!WindowShouldClose())
    {
        // press enter to change all the behaviours at once
        if (IsKeyPressed(KEY_ENTER)) {
            initialize_forces_matrix();
        }

        if (IsKeyPressed(KEY_D)) {
            DEBUG = !DEBUG;
        }

        update_simulation(timestep);

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

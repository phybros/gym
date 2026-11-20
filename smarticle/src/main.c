#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <omp.h>

const int screenWidth = 800;
const int screenHeight = 800;

// how many total particles
#define NUM_PARTICLES 5000
// how many different species
#define NUM_SPECIES 8

#define GRID_SIZE 20

int DEBUG = 0;
float max_range = 0.05;
float timestep = 0.004;
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
int grid[GRID_SIZE][GRID_SIZE][NUM_PARTICLES];
int particles_per_cell[GRID_SIZE][GRID_SIZE];

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

    char label[32];

    float grid_factor = screenWidth / (float)GRID_SIZE;

    // this is so gross but works OK
    for (int x = 0; x < GRID_SIZE; x++) {
        if (x > 0) DrawLine(x * grid_factor, 0, x * grid_factor, screenHeight, GRAY);
        for (int y = 0; y < GRID_SIZE; y++) {
            if (y > 0) DrawLine(0, y * grid_factor, screenWidth, y * grid_factor, GRAY);

            int pcount = particles_per_cell[x][y];

            sprintf(label, "%d, %d\n%d", x, y, pcount);        
            DrawText(label, x * grid_factor, y * grid_factor, 10, WHITE);

            
        }
    }
}

int get_cell_index(float position) {
    int result = (int)floorf(position / max_range);
    if (result >= GRID_SIZE) result = 0;
    if (result < 0) result = GRID_SIZE - 1;
    return result;
}

void update_grid_cells() {
    // grid cells are max_range units wide and high
    
    // zero out the grid cell index
    for (int b = 0; b < GRID_SIZE; b++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            particles_per_cell[b][c] = 0;
        }
    }

    for (int i = 0; i < NUM_PARTICLES; i++) {
        int x = get_cell_index(p.positions[i].x);
        int y = get_cell_index(p.positions[i].y);

        grid[x][y][particles_per_cell[x][y]] = i;
        particles_per_cell[x][y]++;
    }
}

float force(float r, float attraction) {
    float beta = 0.3;

    if (r < beta) {
        return r / beta - 1;
    } else if (beta < r && r < 1) {
        return attraction * (1 - fabsf(2 * r - 1 - beta) / (1 - beta));
    } else {
        return 0.0;
    }
}

int get_nearby_particles(int cell_x, int cell_y, int *nearby_particles) {
    int num_nearby_particles = 0;

    // naive solution: just return all particles
    {
        // for (int i = 0; i < NUM_PARTICLES; i++) {
        //     nearby_particles[num_nearby_particles] = i;
        //     num_nearby_particles++;
        // }
    }

    // spatial grid?
    {
        // figure out which grid cell the particle is in
        // return all the particle ids for the 8 cells around it
        //
        // i need an array of all the cells which gets refreshed every timestep
        int max_cell = GRID_SIZE - 1; // TODO: calculate this instead of hardcoding 9

        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                int check_cell_x = cell_x + x;
                int check_cell_y = cell_y + y;

                // because we only check 1 cell in every direction, we can hardcode max_cell and 0
                if (check_cell_x < 0) {
                    check_cell_x = max_cell;
                }
                if (check_cell_y < 0) {
                    check_cell_y = max_cell;
                }
                if (check_cell_x > max_cell) {
                    check_cell_x = 0;
                }
                if (check_cell_y > max_cell) {
                    check_cell_y = 0;
                }

                for (int pp = 0; pp < particles_per_cell[check_cell_x][check_cell_y]; pp++) {
                    nearby_particles[num_nearby_particles] = grid[check_cell_x][check_cell_y][pp];
                    num_nearby_particles++;
                }
            }
        }
    }

    return num_nearby_particles;
}

void update_simulation(float delta) {
    float friction = powf(0.5f, delta / 0.04f);

    // update acceleration based on attraction forces
    // update velocity based on acceleration
    // reduce movement by friction else particles will fly away forever
    // update particle positions based on velocities

    #pragma omp parallel for schedule(dynamic, 64)
    for (int i = 0; i < NUM_PARTICLES; i++) {
        int nearby_particles[NUM_PARTICLES];

        // always start from 0
        Vector2 acceleration = {0};

        int my_cell_x = get_cell_index(p.positions[i].x);
        int my_cell_y = get_cell_index(p.positions[i].y);

        int n = get_nearby_particles(my_cell_x, my_cell_y, nearby_particles);

        for (int j = 0; j < n; j++) {
            int o = nearby_particles[j];

            if (i == o) continue;

            Vector2 d = Vector2Subtract(p.positions[o], p.positions[i]);
            if (d.x >  0.5f) d.x -= 1.0f; else if (d.x < -0.5f) d.x += 1.0f;
            if (d.y >  0.5f) d.y -= 1.0f; else if (d.y < -0.5f) d.y += 1.0f;

            // skip particles that are out of range with a "cheaper" check
            float lsqr = Vector2LengthSqr(d);
            if (lsqr > max_range * max_range) continue;

            float r = sqrtf(lsqr);

            if (r <= 0) continue;

            Vector2 dirToOther = (Vector2) {
                .x = d.x / r,
                .y = d.y / r,
            };

            float attraction = attractions[p.colors[i]][p.colors[o]];
            float f = force(r / max_range, attraction);
            acceleration.x += dirToOther.x * f;
            acceleration.y += dirToOther.y * f;
        }

        acceleration = Vector2Scale(acceleration, max_range * force_scale_factor);

        p.velocities[i] = Vector2Scale(p.velocities[i], friction);
        p.velocities[i] = Vector2Add(p.velocities[i], Vector2Scale(acceleration, delta));
    }
    

    for (int i = 0; i < NUM_PARTICLES; i++) {
        // update positions based on velocity
        p.positions[i].x += p.velocities[i].x * delta;
        p.positions[i].y += p.velocities[i].y * delta;

        p.positions[i].x -= floorf(p.positions[i].x);
        p.positions[i].y -= floorf(p.positions[i].y);
    }

    update_grid_cells();
}

void render() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
       // DrawCircle(p.positions[i].x * screenWidth, p.positions[i].y * screenHeight, 2, species_colors[p.colors[i]]);
       DrawPixel(p.positions[i].x * screenWidth, p.positions[i].y * screenHeight, species_colors[p.colors[i]]);
    }

    draw_grid();
}

int main(int argc, char ** argv)
{
    const int PARTICLES_PER_SPECIES = (int)floorf(NUM_PARTICLES / NUM_SPECIES);

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

    // initialize all the particles
    float offset = 1.0f/(float)NUM_SPECIES;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // random positions
        // p.positions[i] = (Vector2){
        //     .x = randf_range(offset - 1.0f/(float)NUM_SPECIES, offset),
        //     .y = randf_range(offset - 1.0f/(float)NUM_SPECIES, offset),
        // };
        p.positions[i] = (Vector2) {
            .x = randf_range(0, 1),
            .y = randf_range(0, 1),
        };

        p.colors[i] = i % NUM_SPECIES;

        if (i % PARTICLES_PER_SPECIES == 0 && i > 0) {
            offset += 1.0f/(float)NUM_SPECIES;
        }
    }

    initialize_forces_matrix();

    update_grid_cells();
    double t = GetTime();
    double t_sim = t;
    double t_render = t;

    char t_sim_label[32];
    char t_render_label[32];

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

        t = GetTime();
        update_simulation(timestep);
        t_sim = GetTime() - t;

        // redner the particles!
        BeginDrawing();
            t = GetTime();
            ClearBackground(BLACK);
            render();
            if (DEBUG) DrawFPS(2, 2);
            t_render = GetTime() - t;

            if (DEBUG) {
                sprintf(t_sim_label, "sim: %f", t_sim * 1000.0f);
                sprintf(t_render_label, "ren: %f", t_render * 1000.0f);
                DrawText(t_sim_label, 2, 20, 20, GREEN);
                DrawText(t_render_label, 2, 40, 20, GREEN);
            }
        EndDrawing();

    }

    CloseWindow();
    return 0;
}

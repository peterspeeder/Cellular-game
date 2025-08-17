#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define EMPTY 0
#define SAND 1
#define WATER 2
#define STONE 3
#define ACID 4
#define GAS 5
#define FIRE 6
#define ACID_GAS 7
#define STEAM 8
#define RAIN 9
#define DIRT 10
#define GRASS_SEED 11
#define GRASS 12

// Define acid color stages (from light to dark green)
Color acidColors[5] = {
    (Color){100, 200, 100, 200},   // Light green (fresh acid)
    (Color){80, 180, 80, 200},     //
    (Color){60, 160, 60, 200},     //
    (Color){40, 140, 40, 200},     //
    (Color){20, 120, 20, 200}      // Dark green (fully converted acid)
};

// Define acid gas colors (lighter to darker)
Color acidGasColors[3] = {
    (Color){150, 255, 150, 100},   // Light green
    (Color){100, 220, 100, 150},   // Medium green
    (Color){50, 180, 50, 200}      // Dark green
};

// Define fire colors with more variations
Color fireColors[5] = {
    (Color){255, 100, 0, 200},    // Orange
    (Color){255, 50, 0, 200},     // Red-orange
    (Color){255, 0, 0, 200},      // Red
    (Color){255, 150, 0, 200},    // Yellow-orange
    (Color){255, 200, 0, 200}     // Yellow
};

// Define steam colors (light to dark gray)
Color steamColors[3] = {
    (Color){220, 220, 220, 200},   // Light gray
    (Color){200, 200, 200, 150},   // Medium gray
    (Color){180, 180, 180, 100}    // Dark gray
};

// Define rain droplet color
Color rainColor = (Color){50, 150, 255, 220};

// Physics functions
bool updateFalling(int gridHeight, int gridWidth, int** grid, int x, int y, int element) {
    if (y + 1 < gridHeight && grid[y + 1][x] == EMPTY) {
        grid[y + 1][x] = element;
        grid[y][x] = EMPTY;
        return true;
    }
    if (y + 1 < gridHeight && (grid[y + 1][x] == WATER || grid[y + 1][x] == GAS || grid[y + 1][x] == ACID_GAS)) {
        int temp = grid[y + 1][x];
        grid[y + 1][x] = element;
        grid[y][x] = temp;
        return true;
    }

    int dir = (GetRandomValue(0, 1) == 0) ? -1 : 1;
    if (x + dir >= 0 && x + dir < gridWidth && y + 1 < gridHeight && grid[y + 1][x + dir] == EMPTY) {
        grid[y + 1][x + dir] = element;
        grid[y][x] = EMPTY;
        return true;
    }

    int otherDir = -dir;
    if (x + otherDir >= 0 && x + otherDir < gridWidth && y + 1 < gridHeight && grid[y + 1][x + otherDir] == EMPTY) {
        grid[y + 1][x + otherDir] = element;
        grid[y][x] = EMPTY;
        return true;
    }

    return false;
}

bool updateSand(int gridHeight, int gridWidth, int** grid, int x, int y) {
    return updateFalling(gridHeight, gridWidth, grid, x, y, SAND);
}

bool updateDirt(int gridHeight, int gridWidth, int** grid, int x, int y) {
    return updateFalling(gridHeight, gridWidth, grid, x, y, DIRT);
}

bool updateWater(int gridHeight, int gridWidth, int** grid, int x, int y) {
    if (y - 1 >= 0 && grid[y - 1][x] == SAND) {
        grid[y - 1][x] = WATER;
        grid[y][x] = SAND;
        return true;
    }

    if (y + 1 < gridHeight && (grid[y + 1][x] == EMPTY || grid[y + 1][x] == GAS || grid[y + 1][x] == ACID_GAS)) {
        int temp = grid[y + 1][x];
        grid[y + 1][x] = WATER;
        grid[y][x] = temp;
        return true;
    }

    int dir = (GetRandomValue(0, 1) == 0) ? -1 : 1;
    if (x + dir >= 0 && x + dir < gridWidth && y + 1 < gridHeight && (grid[y + 1][x + dir] == EMPTY || grid[y + 1][x + dir] == GAS || grid[y + 1][x + dir] == ACID_GAS)) {
        int temp = grid[y + 1][x + dir];
        grid[y + 1][x + dir] = WATER;
        grid[y][x] = temp;
        return true;
    }

    int otherDir = -dir;
    if (x + otherDir >= 0 && x + otherDir < gridWidth && y + 1 < gridHeight && (grid[y + 1][x + otherDir] == EMPTY || grid[y + 1][x + otherDir] == GAS || grid[y + 1][x + otherDir] == ACID_GAS)) {
        int temp = grid[y + 1][x + otherDir];
        grid[y + 1][x + otherDir] = WATER;
        grid[y][x] = temp;
        return true;
    }

    if (x + dir >= 0 && x + dir < gridWidth && (grid[y][x + dir] == EMPTY || grid[y][x + dir] == GAS || grid[y][x + dir] == ACID_GAS)) {
        int temp = grid[y][x + dir];
        grid[y][x + dir] = WATER;
        grid[y][x] = temp;
        return true;
    }

    if (x + otherDir >= 0 && x + otherDir < gridWidth && (grid[y][x + otherDir] == EMPTY || grid[y][x + otherDir] == GAS || grid[y][x + otherDir] == ACID_GAS)) {
        int temp = grid[y][x + otherDir];
        grid[y][x + otherDir] = WATER;
        grid[y][x] = temp;
        return true;
    }

    return false;
}

float findNearestAcidDistance(int gridHeight, int gridWidth, int** grid, int x, int y) {
    float minDistance = INFINITY;

    for (int ay = 0; ay < gridHeight; ay++) {
        for (int ax = 0; ax < gridWidth; ax++) {
            if (grid[ay][ax] == ACID) {
                float dist = sqrtf((ax - x) * (ax - x) + (ay - y) * (ay - y));
                if (dist < minDistance) {
                    minDistance = dist;
                }
            }
        }
    }

    return minDistance;
}

bool updateAcid(int gridHeight, int gridWidth, int** grid,
                int** acidStage, int** acidTimer, int** acidGasCooldown,
                int x, int y) {
    acidTimer[y][x]++;

    if (GetRandomValue(0, 100) < 2 && acidTimer[y][x] > 300) {
        if (grid[y][x] == ACID) {
            grid[y][x] = ACID_GAS;
            acidTimer[y][x] = GetRandomValue(60, 180);
            acidStage[y][x] = GetRandomValue(0, 2);
            acidGasCooldown[y][x] = 10;
            return true;
        }
    }

    int dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        int nx = x + dirs[i][0];
        int ny = y + dirs[i][1];
        if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight &&
            (grid[ny][nx] == SAND || grid[ny][nx] == STONE)) {
            grid[ny][nx] = EMPTY;
            return true;
        }
    }

    // Convert adjacent water to acid with gradual color change
    for (int i = 0; i < 4; i++) {
        int nx = x + dirs[i][0];
        int ny = y + dirs[i][1];
        if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight && grid[ny][nx] == WATER) {
            float dist = findNearestAcidDistance(gridHeight, gridWidth, grid, nx, ny);
            int conversionRate = (int)(30.0f / (1.0f + dist));
            
            if (GetRandomValue(0, conversionRate) == 0) {
                if (acidStage[ny][nx] < 4) {
                    acidStage[ny][nx]++;
                    return true;
                } else {
                    grid[ny][nx] = ACID;
                    acidStage[ny][nx] = 0;
                    acidTimer[ny][nx] = 0;
                    return true;
                }
            }
        }
    }

    if (y + 1 < gridHeight && (grid[y + 1][x] == EMPTY || grid[y + 1][x] == WATER || grid[y + 1][x] == STONE)) {
        grid[y + 1][x] = ACID;
        acidStage[y + 1][x] = 0;
        acidTimer[y + 1][x] = acidTimer[y][x];
        grid[y][x] = EMPTY;
        acidStage[y][x] = 0;
        acidTimer[y][x] = 0;
        return true;
    }

    int dir = (GetRandomValue(0, 1) == 0) ? -1 : 1;
    if (x + dir >= 0 && x + dir < gridWidth && y + 1 < gridHeight &&
        (grid[y + 1][x + dir] == EMPTY || grid[y + 1][x + dir] == WATER || grid[y + 1][x + dir] == STONE)) {
        grid[y + 1][x + dir] = ACID;
        acidStage[y + 1][x + dir] = 0;
        acidTimer[y + 1][x + dir] = acidTimer[y][x];
        grid[y][x] = EMPTY;
        acidStage[y][x] = 0;
        acidTimer[y][x] = 0;
        return true;
    }

    int otherDir = -dir;
    if (x + otherDir >= 0 && x + otherDir < gridWidth && y + 1 < gridHeight &&
        (grid[y + 1][x + otherDir] == EMPTY || grid[y + 1][x + otherDir] == WATER || grid[y + 1][x + otherDir] == STONE)) {
        grid[y + 1][x + otherDir] = ACID;
        acidStage[y + 1][x + otherDir] = 0;
        acidTimer[y + 1][x + otherDir] = acidTimer[y][x];
        grid[y][x] = EMPTY;
        acidStage[y][x] = 0;
        acidTimer[y][x] = 0;
        return true;
    }

    if (x + dir >= 0 && x + dir < gridWidth &&
        (grid[y][x + dir] == EMPTY || grid[y][x + dir] == WATER || grid[y][x + dir] == STONE)) {
        grid[y][x + dir] = ACID;
        acidStage[y][x + dir] = 0;
        acidTimer[y][x + dir] = acidTimer[y][x];
        grid[y][x] = EMPTY;
        acidStage[y][x] = 0;
        acidTimer[y][x] = 0;
        return true;
    }

    if (x + otherDir >= 0 && x + otherDir < gridWidth &&
        (grid[y][x + otherDir] == EMPTY || grid[y][x + otherDir] == WATER || grid[y][x + otherDir] == STONE)) {
        grid[y][x + otherDir] = ACID;
        acidStage[y][x + otherDir] = 0;
        acidTimer[y][x + otherDir] = acidTimer[y][x];
        grid[y][x] = EMPTY;
        acidStage[y][x] = 0;
        acidTimer[y][x] = 0;
        return true;
    }

    if (acidTimer[y][x] > 1200) {
        grid[y][x] = EMPTY;
        acidStage[y][x] = 0;
        acidTimer[y][x] = 0;
        return true;
    }

    return false;
}

bool updateAcidGas(int gridHeight, int gridWidth, int** grid,
                   int** acidTimer, int** acidStage, int** acidGasCooldown,
                   int x, int y) {
    acidTimer[y][x]--;

    if (acidGasCooldown[y][x] > 0) {
        acidGasCooldown[y][x]--;
    }

    if (acidGasCooldown[y][x] == 0) {
        if (y - 1 >= 0 && grid[y - 1][x] == EMPTY) {
            grid[y - 1][x] = ACID_GAS;
            acidTimer[y - 1][x] = acidTimer[y][x];
            acidStage[y - 1][x] = acidStage[y][x];
            acidGasCooldown[y - 1][x] = 5;
            grid[y][x] = EMPTY;
            acidTimer[y][x] = 0;
            acidStage[y][x] = 0;
            acidGasCooldown[y][x] = 0;
            return true;
        }

        int dir = (GetRandomValue(0, 1) == 0) ? -1 : 1;
        if (x + dir >= 0 && x + dir < gridWidth && grid[y][x + dir] == EMPTY) {
            grid[y][x + dir] = ACID_GAS;
            acidTimer[y][x + dir] = acidTimer[y][x];
            acidStage[y][x + dir] = acidStage[y][x];
            acidGasCooldown[y][x + dir] = 5;
            grid[y][x] = EMPTY;
            acidTimer[y][x] = 0;
            acidStage[y][x] = 0;
            acidGasCooldown[y][x] = 0;
            return true;
        }
    }

    if (acidTimer[y][x] <= 0) {
        grid[y][x] = EMPTY;
        acidTimer[y][x] = 0;
        acidStage[y][x] = 0;
        acidGasCooldown[y][x] = 0;
        return true;
    }

    return false;
}

bool updateGas(int gridHeight, int gridWidth, int** grid, int** gasTimer, int** gasCooldown, int x, int y, int** fireTimer) {
    int dirs[4][2] = {{0,1}, {1,0}, {0,-1}, {-1,0}};
    for (int i = 0; i < 4; i++) {
        int nx = x + dirs[i][0];
        int ny = y + dirs[i][1];
        if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight && grid[ny][nx] == FIRE) {
            grid[y][x] = FIRE;
            fireTimer[y][x] = 0;
            gasTimer[y][x] = 0;
            gasCooldown[y][x] = 0;
            return true;
        }
    }

    gasTimer[y][x]++;

    if (gasCooldown[y][x] > 0) {
        gasCooldown[y][x]--;
    }

    if (gasCooldown[y][x] > 0) {
        return false;
    }

    if (y - 1 >= 0 && grid[y - 1][x] == EMPTY) {
        if (GetRandomValue(0, 100) < 60) {
            grid[y - 1][x] = GAS;
            gasTimer[y - 1][x] = gasTimer[y][x];
            gasCooldown[y - 1][x] = 2;
            grid[y][x] = EMPTY;
            gasTimer[y][x] = 0;
            gasCooldown[y][x] = 0;
            return true;
        }
    }

    int directions[8][2] = {
        {-1, 0}, {1, 0}, {-1, -1}, {1, -1},
        {-1, 1}, {1, 1}, {0, -1}, {0, 1}
    };

    for (int i = 0; i < 8; i++) {
        int swapIndex = GetRandomValue(0, 7);
        int tempX = directions[i][0];
        int tempY = directions[i][1];
        directions[i][0] = directions[swapIndex][0];
        directions[i][1] = directions[swapIndex][1];
        directions[swapIndex][0] = tempX;
        directions[swapIndex][1] = tempY;
    }

    for (int i = 0; i < 8; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];
        int nx = x + dx;
        int ny = y + dy;

        if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight && grid[ny][nx] == EMPTY) {
            grid[ny][nx] = GAS;
            gasTimer[ny][nx] = gasTimer[y][x];
            gasCooldown[ny][nx] = 2;
            grid[y][x] = EMPTY;
            gasTimer[y][x] = 0;
            gasCooldown[y][x] = 0;
            return true;
        }
    }

    if (gasTimer[y][x] > 600) {
        grid[y][x] = EMPTY;
        gasTimer[y][x] = 0;
        gasCooldown[y][x] = 0;
        return true;
    }

    gasCooldown[y][x] = 2;
    return false;
}

bool updateFire(int gridHeight, int gridWidth, int** grid, int** fireTimer, int** steamTimer, int x, int y) {
    fireTimer[y][x]++;
    
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                if (grid[ny][nx] == WATER) {
                    grid[ny][nx] = STEAM;
                    steamTimer[ny][nx] = 0;
                }
                else if (grid[ny][nx] == STEAM) {
                    grid[ny][nx] = EMPTY;
                    steamTimer[ny][nx] = 0;
                }
            }
        }
    }

    if (GetRandomValue(0, 100) < 50) {
        int moveX = x + GetRandomValue(-1, 1);
        int moveY = y - 1;

        if (moveX >= 0 && moveX < gridWidth && moveY >= 0 && moveY < gridHeight) {
            if (grid[moveY][moveX] == EMPTY) {
                grid[moveY][moveX] = FIRE;
                fireTimer[moveY][moveX] = fireTimer[y][x];
                grid[y][x] = EMPTY;
                fireTimer[y][x] = 0;
                return true;
            }
        }
    }

    if (fireTimer[y][x] > 30) {
        grid[y][x] = EMPTY;
        fireTimer[y][x] = 0;
        return true;
    }

    return false;
}

bool updateSteam(int gridHeight, int gridWidth, int** grid, int** steamTimer, int x, int y) {
    steamTimer[y][x]++;
    
    if (steamTimer[y][x] > 1000 && steamTimer[y][x] < 1100 && 
        y < gridHeight/2 && GetRandomValue(0, 100) < 2) {
        if (y+1 < gridHeight && grid[y+1][x] == EMPTY) {
            grid[y+1][x] = RAIN;
            return true;
        }
    }
    
    int riseChance = 70;
    if (GetRandomValue(0, 100) < riseChance && y > 0) {
        int dir = GetRandomValue(0, 2) - 1;
        int newX = x + dir;
        int newY = y - 1;
        
        if (newX >= 0 && newX < gridWidth && newY >= 0) {
            if (grid[newY][newX] == EMPTY || grid[newY][newX] == WATER) {
                int targetMaterial = grid[newY][newX];
                grid[newY][newX] = STEAM;
                steamTimer[newY][newX] = steamTimer[y][x];
                
                grid[y][x] = targetMaterial;
                steamTimer[y][x] = 0;
                return true;
            }
        }
    }
    
    if (GetRandomValue(0, 100) < 40) {
        int dir = (GetRandomValue(0, 1) == 0) ? -1 : 1;
        int newX = x + dir;
        
        if (newX >= 0 && newX < gridWidth) {
            if (grid[y][newX] == EMPTY || grid[y][newX] == WATER) {
                int targetMaterial = grid[y][newX];
                grid[y][newX] = STEAM;
                steamTimer[y][newX] = steamTimer[y][x];
                
                grid[y][x] = targetMaterial;
                steamTimer[y][x] = 0;
                return true;
            }
        }
    }
    
    if (steamTimer[y][x] > 1100) {
        grid[y][x] = EMPTY;
        steamTimer[y][x] = 0;
        return true;
    }
    
    return false;
}

bool updateRain(int gridHeight, int gridWidth, int** grid, int x, int y) {
    if (y+1 < gridHeight) {
        if (grid[y+1][x] == EMPTY) {
            grid[y+1][x] = RAIN;
            grid[y][x] = EMPTY;
            return true;
        }
        else if (grid[y+1][x] != RAIN) {
            grid[y][x] = WATER;
            return true;
        }
    }
    else {
        grid[y][x] = WATER;
        return true;
    }
    
    return false;
}

bool updateGrassSeed(int gridHeight, int gridWidth, int** grid, int x, int y) {
    // Falling behavior
    if (y+1 < gridHeight) {
        if (grid[y+1][x] == EMPTY) {
            grid[y+1][x] = GRASS_SEED;
            grid[y][x] = EMPTY;
            return true;
        }
        else if (grid[y+1][x] == WATER || grid[y+1][x] == GAS || grid[y+1][x] == ACID_GAS) {
            int temp = grid[y+1][x];
            grid[y+1][x] = GRASS_SEED;
            grid[y][x] = temp;
            return true;
        }
    }
    
    // Check if seed is on top of dirt
    if (y+1 < gridHeight && grid[y+1][x] == DIRT) {
        // Convert seed to grass
        grid[y][x] = GRASS;
        
        // Grow grass upward (max 6 cells)
        int currentY = y-1;
        int grassCount = 0;
        while (grassCount < 6 && currentY >= 0) {
            if (grid[currentY][x] == EMPTY) {
                grid[currentY][x] = GRASS;
                grassCount++;
                currentY--;
            } else {
                break;
            }
        }
        return true;
    }
    
    return false;
}

int main() {
    const int initialWidth = 800;
    const int initialHeight = 600;
    const int gridSize = 10;
    const int buttonWidth = 80;
    const int buttonHeight = 40;
    const int buttonSpacing = 10;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(initialWidth, initialHeight, "Sand Simulation with Elements");

    int gridWidth = (initialWidth - 150) / gridSize;
    int gridHeight = initialHeight / gridSize;

    int **grid = (int **)malloc(gridHeight * sizeof(int *));
    int **acidStage = (int **)malloc(gridHeight * sizeof(int *));
    int **acidTimer = (int **)malloc(gridHeight * sizeof(int *));
    int **fireTimer = (int **)malloc(gridHeight * sizeof(int *));
    int **gasTimer = (int **)malloc(gridHeight * sizeof(int *));
    int **gasCooldown = (int **)malloc(gridHeight * sizeof(int *));
    int **acidGasCooldown = (int **)malloc(gridHeight * sizeof(int *));
    int **steamTimer = (int **)malloc(gridHeight * sizeof(int *));
    bool **updated = (bool **)malloc(gridHeight * sizeof(bool *));

    for (int y = 0; y < gridHeight; y++) {
        grid[y] = (int *)malloc(gridWidth * sizeof(int));
        acidStage[y] = (int *)malloc(gridWidth * sizeof(int));
        acidTimer[y] = (int *)malloc(gridWidth * sizeof(int));
        fireTimer[y] = (int *)malloc(gridWidth * sizeof(int));
        gasTimer[y] = (int *)malloc(gridWidth * sizeof(int));
        gasCooldown[y] = (int *)malloc(gridWidth * sizeof(int));
        acidGasCooldown[y] = (int *)malloc(gridWidth * sizeof(int));
        steamTimer[y] = (int *)malloc(gridWidth * sizeof(int));
        updated[y] = (bool *)malloc(gridWidth * sizeof(bool));
        for (int x = 0; x < gridWidth; x++) {
            grid[y][x] = EMPTY;
            acidStage[y][x] = 0;
            acidTimer[y][x] = 0;
            fireTimer[y][x] = 0;
            gasTimer[y][x] = 0;
            gasCooldown[y][x] = 0;
            acidGasCooldown[y][x] = 0;
            steamTimer[y][x] = 0;
            updated[y][x] = false;
        }
    }

    Rectangle sandButton = { buttonSpacing, buttonSpacing, buttonWidth, buttonHeight };
    Rectangle waterButton = { buttonSpacing, buttonSpacing*2 + buttonHeight, buttonWidth, buttonHeight };
    Rectangle stoneButton = { buttonSpacing, buttonSpacing*3 + buttonHeight*2, buttonWidth, buttonHeight };
    Rectangle acidButton = { buttonSpacing, buttonSpacing*4 + buttonHeight*3, buttonWidth, buttonHeight };
    Rectangle gasButton = { buttonSpacing, buttonSpacing*5 + buttonHeight*4, buttonWidth, buttonHeight };
    Rectangle fireButton = { buttonSpacing, buttonSpacing*6 + buttonHeight*5, buttonWidth, buttonHeight };
    Rectangle eraseButton = { buttonSpacing, buttonSpacing*7 + buttonHeight*6, buttonWidth, buttonHeight };
    Rectangle eraseAllButton = { buttonSpacing, buttonSpacing*8 + buttonHeight*7, buttonWidth, buttonHeight };
    Rectangle dirtButton = { buttonSpacing, buttonSpacing*9 + buttonHeight*8, buttonWidth, buttonHeight };
    Rectangle grassSeedButton = { buttonSpacing, buttonSpacing*10 + buttonHeight*9, buttonWidth, buttonHeight };

    int currentMaterial = SAND;
    int brushSize = 3;
    int framesCounter = 0;
    int evaporationCounter = 0;
    const int evaporationTime = 20 * 60;
    const int maxEvaporationsPerFrame = 10;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            int newWidth = GetScreenWidth();
            int newHeight = GetScreenHeight();
            int newGridWidth = (newWidth - 150) / gridSize;
            int newGridHeight = newHeight / gridSize;

            if (newGridWidth != gridWidth || newGridHeight != gridHeight) {
                for (int y = 0; y < gridHeight; y++) {
                    free(grid[y]);
                    free(acidStage[y]);
                    free(acidTimer[y]);
                    free(fireTimer[y]);
                    free(gasTimer[y]);
                    free(gasCooldown[y]);
                    free(acidGasCooldown[y]);
                    free(steamTimer[y]);
                    free(updated[y]);
                }
                free(grid);
                free(acidStage);
                free(acidTimer);
                free(fireTimer);
                free(gasTimer);
                free(gasCooldown);
                free(acidGasCooldown);
                free(steamTimer);
                free(updated);

                gridWidth = newGridWidth;
                gridHeight = newGridHeight;

                grid = (int **)malloc(gridHeight * sizeof(int *));
                acidStage = (int **)malloc(gridHeight * sizeof(int *));
                acidTimer = (int **)malloc(gridHeight * sizeof(int *));
                fireTimer = (int **)malloc(gridHeight * sizeof(int *));
                gasTimer = (int **)malloc(gridHeight * sizeof(int *));
                gasCooldown = (int **)malloc(gridHeight * sizeof(int *));
                acidGasCooldown = (int **)malloc(gridHeight * sizeof(int *));
                steamTimer = (int **)malloc(gridHeight * sizeof(int *));
                updated = (bool **)malloc(gridHeight * sizeof(bool *));

                for (int y = 0; y < gridHeight; y++) {
                    grid[y] = (int *)malloc(gridWidth * sizeof(int));
                    acidStage[y] = (int *)malloc(gridWidth * sizeof(int));
                    acidTimer[y] = (int *)malloc(gridWidth * sizeof(int));
                    fireTimer[y] = (int *)malloc(gridWidth * sizeof(int));
                    gasTimer[y] = (int *)malloc(gridWidth * sizeof(int));
                    gasCooldown[y] = (int *)malloc(gridWidth * sizeof(int));
                    acidGasCooldown[y] = (int *)malloc(gridWidth * sizeof(int));
                    steamTimer[y] = (int *)malloc(gridWidth * sizeof(int));
                    updated[y] = (bool *)malloc(gridWidth * sizeof(bool));
                    for (int x = 0; x < gridWidth; x++) {
                        grid[y][x] = EMPTY;
                        acidStage[y][x] = 0;
                        acidTimer[y][x] = 0;
                        fireTimer[y][x] = 0;
                        gasTimer[y][x] = 0;
                        gasCooldown[y][x] = 0;
                        acidGasCooldown[y][x] = 0;
                        steamTimer[y][x] = 0;
                        updated[y][x] = false;
                    }
                }
            }
        }

        Vector2 mousePos = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, sandButton)) {
                currentMaterial = SAND;
            }
            else if (CheckCollisionPointRec(mousePos, waterButton)) {
                currentMaterial = WATER;
            }
            else if (CheckCollisionPointRec(mousePos, stoneButton)) {
                currentMaterial = STONE;
            }
            else if (CheckCollisionPointRec(mousePos, acidButton)) {
                currentMaterial = ACID;
            }
            else if (CheckCollisionPointRec(mousePos, gasButton)) {
                currentMaterial = GAS;
            }
            else if (CheckCollisionPointRec(mousePos, fireButton)) {
                currentMaterial = FIRE;
            }
            else if (CheckCollisionPointRec(mousePos, eraseButton)) {
                currentMaterial = EMPTY;
            }
            else if (CheckCollisionPointRec(mousePos, eraseAllButton)) {
                for (int y = 0; y < gridHeight; y++) {
                    for (int x = 0; x < gridWidth; x++) {
                        grid[y][x] = EMPTY;
                        acidStage[y][x] = 0;
                        acidTimer[y][x] = 0;
                        fireTimer[y][x] = 0;
                        gasTimer[y][x] = 0;
                        gasCooldown[y][x] = 0;
                        acidGasCooldown[y][x] = 0;
                        steamTimer[y][x] = 0;
                    }
                }
            }
            else if (CheckCollisionPointRec(mousePos, dirtButton)) {
                currentMaterial = DIRT;
            }
            else if (CheckCollisionPointRec(mousePos, grassSeedButton)) {
                currentMaterial = GRASS_SEED;
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && mousePos.x > 150) {
            int gridX = (mousePos.x - 150) / gridSize;
            int gridY = mousePos.y / gridSize;

            if (currentMaterial == FIRE) {
                if (gridY >= 0 && gridY < gridHeight && gridX >= 0 && gridX < gridWidth) {
                    grid[gridY][gridX] = FIRE;
                    fireTimer[gridY][gridX] = 0;

                    int flameHeight = brushSize * 2;
                    for (int i = 1; i <= flameHeight; i++) {
                        int flameY = gridY - i;
                        if (flameY < 0) break;

                        int intensity = 100 - (i * 100 / flameHeight);
                        if (intensity < 0) intensity = 0;

                        if (GetRandomValue(0, 100) < intensity) {
                            if (grid[flameY][gridX] == EMPTY) {
                                grid[flameY][gridX] = FIRE;
                                fireTimer[flameY][gridX] = 0;
                            }
                        }
                    }
                }
            }
            else if (currentMaterial == GRASS_SEED) {
                // Place only one seed at the mouse position
                if (gridX >= 0 && gridX < gridWidth && gridY >= 0 && gridY < gridHeight) {
                    // Only place if cell is empty
                    if (grid[gridY][gridX] == EMPTY) {
                        grid[gridY][gridX] = GRASS_SEED;
                    }
                }
            }
            else {
                for (int y = gridY - brushSize/2; y <= gridY + brushSize/2; y++) {
                    for (int x = gridX - brushSize/2; x <= gridX + brushSize/2; x++) {
                        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
                            if (currentMaterial == GAS) {
                                if (grid[y][x] != STONE && grid[y][x] != ACID && grid[y][x] != GRASS_SEED) {
                                    grid[y][x] = GAS;
                                    gasTimer[y][x] = 0;
                                    gasCooldown[y][x] = 0;
                                }
                            }
                            else if (currentMaterial == ACID) {
                                if (grid[y][x] != GRASS_SEED) {
                                    grid[y][x] = ACID;
                                    acidStage[y][x] = 0;
                                    acidTimer[y][x] = 0;
                                    acidGasCooldown[y][x] = 0;
                                }
                            }
                            else if (currentMaterial == EMPTY) {
                                grid[y][x] = EMPTY;
                                acidStage[y][x] = 0;
                                acidTimer[y][x] = 0;
                                fireTimer[y][x] = 0;
                                gasTimer[y][x] = 0;
                                gasCooldown[y][x] = 0;
                                acidGasCooldown[y][x] = 0;
                                steamTimer[y][x] = 0;
                            }
                            else if (currentMaterial != GRASS_SEED) {
                                grid[y][x] = currentMaterial;
                            }
                        }
                    }
                }
            }
        }

        int wheelMove = GetMouseWheelMove();
        if (wheelMove != 0) {
            brushSize += wheelMove;
            if (brushSize < 1) brushSize = 1;
            if (brushSize > 10) brushSize = 10;
        }

        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                updated[y][x] = false;
            }
        }

        for (int y = gridHeight - 1; y >= 0; y--) {
            for (int x = 0; x < gridWidth; x++) {
                if (!updated[y][x]) {
                    if (grid[y][x] == SAND) {
                        if (updateSand(gridHeight, gridWidth, grid, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == WATER) {
                        if (updateWater(gridHeight, gridWidth, grid, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == ACID) {
                        if (updateAcid(gridHeight, gridWidth, grid, acidStage, acidTimer, acidGasCooldown, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == GAS) {
                        if (updateGas(gridHeight, gridWidth, grid, gasTimer, gasCooldown, x, y, fireTimer)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == FIRE) {
                        if (updateFire(gridHeight, gridWidth, grid, fireTimer, steamTimer, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == ACID_GAS) {
                        if (updateAcidGas(gridHeight, gridWidth, grid, acidTimer, acidStage, acidGasCooldown, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == STEAM) {
                        if (updateSteam(gridHeight, gridWidth, grid, steamTimer, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == RAIN) {
                        if (updateRain(gridHeight, gridWidth, grid, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == DIRT) {
                        if (updateDirt(gridHeight, gridWidth, grid, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                    else if (grid[y][x] == GRASS_SEED) {
                        if (updateGrassSeed(gridHeight, gridWidth, grid, x, y)) {
                            updated[y][x] = true;
                        }
                    }
                }
            }
        }

        evaporationCounter++;
        if (evaporationCounter >= evaporationTime) {
            evaporationCounter = 0;

            int evaporated = 0;
            for (int y = 0; y < gridHeight && evaporated < maxEvaporationsPerFrame; y++) {
                for (int x = 0; x < gridWidth && evaporated < maxEvaporationsPerFrame; x++) {
                    if (grid[y][x] == ACID && acidTimer[y][x] >= evaporationTime) {
                        grid[y][x] = EMPTY;
                        acidStage[y][x] = 0;
                        acidTimer[y][x] = 0;
                        evaporated++;
                    }
                }
            }
        }

        BeginDrawing();
            ClearBackground((Color){0, 0, 0, 255});

            // Draw brush size in top-right corner
            DrawText(TextFormat("Brush Size: %d", brushSize), GetScreenWidth() - 150, 10, 20, WHITE);

            DrawRectangleRec(sandButton, YELLOW);
            DrawRectangleRec(waterButton, BLUE);
            DrawRectangleRec(stoneButton, DARKGRAY);
            DrawRectangleRec(acidButton, acidColors[0]);
            DrawRectangleRec(gasButton, (Color){180, 180, 180, 200});
            DrawRectangleRec(fireButton, fireColors[0]);
            DrawRectangleRec(eraseButton, RED);
            DrawRectangleRec(eraseAllButton, (Color){200, 100, 100, 255});
            DrawRectangleRec(dirtButton, (Color){139, 69, 19, 255});  // Brown
            DrawRectangleRec(grassSeedButton, (Color){205, 133, 63, 255});  // Light brown

            DrawText("Sand", sandButton.x + 10, sandButton.y + 10, 20, BLACK);
            DrawText("Water", waterButton.x + 10, waterButton.y + 10, 20, BLACK);
            DrawText("Stone", stoneButton.x + 10, stoneButton.y + 10, 20, BLACK);
            DrawText("Acid", acidButton.x + 10, acidButton.y + 10, 20, BLACK);
            DrawText("Gas", gasButton.x + 10, gasButton.y + 10, 20, BLACK);
            DrawText("Fire", fireButton.x + 10, fireButton.y + 10, 20, BLACK);
            DrawText("Erase", eraseButton.x + 10, eraseButton.y + 10, 20, BLACK);
            DrawText("Clear All", eraseAllButton.x + 5, eraseAllButton.y + 10, 20, BLACK);
            DrawText("Dirt", dirtButton.x + 10, dirtButton.y + 10, 20, BLACK);
            DrawText("Seed", grassSeedButton.x + 10, grassSeedButton.y + 10, 20, BLACK);

            if (currentMaterial == SAND) {
                DrawRectangleLinesEx(sandButton, 3, RED);
            } else if (currentMaterial == WATER) {
                DrawRectangleLinesEx(waterButton, 3, RED);
            } else if (currentMaterial == STONE) {
                DrawRectangleLinesEx(stoneButton, 3, RED);
            } else if (currentMaterial == ACID) {
                DrawRectangleLinesEx(acidButton, 3, RED);
            } else if (currentMaterial == GAS) {
                DrawRectangleLinesEx(gasButton, 3, RED);
            } else if (currentMaterial == FIRE) {
                DrawRectangleLinesEx(fireButton, 3, RED);
            } else if (currentMaterial == EMPTY) {
                DrawRectangleLinesEx(eraseButton, 3, WHITE);
            } else if (currentMaterial == DIRT) {
                DrawRectangleLinesEx(dirtButton, 3, WHITE);
            } else if (currentMaterial == GRASS_SEED) {
                DrawRectangleLinesEx(grassSeedButton, 3, WHITE);
            }

            for (int y = 0; y < gridHeight; y++) {
                for (int x = 0; x < gridWidth; x++) {
                    int posX = 150 + x * gridSize;
                    int posY = y * gridSize;

                    if (grid[y][x] == SAND) {
                        DrawRectangle(posX, posY, gridSize, gridSize, YELLOW);
                    } else if (grid[y][x] == WATER) {
                        if (acidStage[y][x] > 0) {
                            float blendRatio = (float)acidStage[y][x] / 4.0f;
                            Color waterColor = {
                                (unsigned char)(0 * (1.0f - blendRatio) + acidColors[acidStage[y][x]].r * blendRatio),
                                (unsigned char)(105 * (1.0f - blendRatio) + acidColors[acidStage[y][x]].g * blendRatio),
                                (unsigned char)(148 * (1.0f - blendRatio) + acidColors[acidStage[y][x]].b * blendRatio),
                                200
                            };
                            DrawRectangle(posX, posY, gridSize, gridSize, waterColor);
                        } else {
                            DrawRectangle(posX, posY, gridSize, gridSize, (Color){0, 105, 148, 200});
                        }
                    } else if (grid[y][x] == STONE) {
                        DrawRectangle(posX, posY, gridSize, gridSize, DARKGRAY);
                    } else if (grid[y][x] == ACID) {
                        float alpha = acidTimer[y][x] > evaporationTime * 0.8f ?
                                     200.0f * (1.0f - (acidTimer[y][x] - evaporationTime * 0.8f) / (evaporationTime * 0.2f)) :
                                     200.0f;
                        Color acidColor = acidColors[0];
                        acidColor.a = alpha;
                        DrawRectangle(posX, posY, gridSize, gridSize, acidColor);
                    } else if (grid[y][x] == GAS) {
                        float alpha = 150.0f * (1.0f - (float)gasTimer[y][x] / 600.0f);
                        if (alpha < 0) alpha = 0;
                        DrawRectangle(posX, posY, gridSize, gridSize, (Color){200, 200, 200, (unsigned char)alpha});
                    } else if (grid[y][x] == FIRE) {
                        int colorIndex = (fireTimer[y][x] + x + y) % 5;
                        DrawRectangle(posX, posY, gridSize, gridSize, fireColors[colorIndex]);
                    } else if (grid[y][x] == ACID_GAS) {
                        Color gasColor = acidGasColors[acidStage[y][x]];
                        float progress = (float)acidTimer[y][x] / 180.0f;
                        gasColor.a = (unsigned char)(gasColor.a * progress);
                        DrawRectangle(posX, posY, gridSize, gridSize, gasColor);
                    } else if (grid[y][x] == STEAM) {
                        int colorIndex = steamTimer[y][x] / 600;
                        if (colorIndex > 2) colorIndex = 2;
                        Color steamColor = steamColors[colorIndex];
                        
                        float progress = (float)steamTimer[y][x] / 1800.0f;
                        steamColor.a = 255 * (1.0f - progress * 0.7f);
                        
                        DrawRectangle(posX, posY, gridSize, gridSize, steamColor);
                    } else if (grid[y][x] == RAIN) {
                        DrawRectangle(posX, posY, gridSize, gridSize, rainColor);
                    } else if (grid[y][x] == DIRT) {
                        DrawRectangle(posX, posY, gridSize, gridSize, (Color){139, 69, 19, 255}); // Brown
                    } else if (grid[y][x] == GRASS_SEED) {
                        DrawRectangle(posX, posY, gridSize, gridSize, (Color){205, 133, 63, 255}); // Light brown
                    } else if (grid[y][x] == GRASS) {
                        DrawRectangle(posX, posY, gridSize, gridSize, (Color){0, 128, 0, 255}); // Green
                    }
                }
            }
        EndDrawing();
    }

    for (int y = 0; y < gridHeight; y++) {
        free(grid[y]);
        free(acidStage[y]);
        free(acidTimer[y]);
        free(fireTimer[y]);
        free(gasTimer[y]);
        free(gasCooldown[y]);
        free(acidGasCooldown[y]);
        free(steamTimer[y]);
        free(updated[y]);
    }
    free(grid);
    free(acidStage);
    free(acidTimer);
    free(fireTimer);
    free(gasTimer);
    free(gasCooldown);
    free(acidGasCooldown);
    free(steamTimer);
    free(updated);

    CloseWindow();
    return 0;
}

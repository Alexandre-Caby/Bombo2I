#include "map.h"

/**
 * function main
 * @brief Main function
 * 
 * @return int
 */
int main() {
    srand(time(NULL));
    Map *map = map_new(MAX_MAP_WIDTH, MAX_MAP_HEIGHT);
    generateMap(map);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window and renderer
    SDL_Window *window = SDL_CreateWindow("Map", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAX_MAP_WIDTH * CELL_SIZE, MAX_MAP_HEIGHT * CELL_SIZE, 0);
    if (!window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_ttf (font rendering library)
    if (TTF_Init() == -1) {
        fprintf(stderr, "Could not initialize SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("/home/alexandre/Bureau/IG2I/La1/Objet connecté/source/Bombo2I/ressources/Minecraft.ttf", 12);
    if (!font) {
        fprintf(stderr, "Could not load font: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x / CELL_SIZE;
                int y = event.button.y / CELL_SIZE;
                if (x > 0 && y > 0 && x < map->width && y < map->height) {
                    // Toggle between START and END points for simplicity
                    if (map->cells[y * map->width + x] == PATH) {
                        placePoint(map, x, y, START);
                    } else if (map->cells[y * map->width + x] == START) {
                        placePoint(map, x, y, END);
                    } else if (map->cells[y * map->width + x] == END) {
                        placePoint(map, x, y, PATH);
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        drawMap(renderer, map, font);

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/**
 * function map_new
 * @brief Create a new map
 * 
 * @param width 
 * @param height 
 * @return Map*
 */
Map* map_new(int width, int height) {
    Map *map = malloc(sizeof(Map));
    if (map == NULL) {
        fprintf(stderr, "Could not allocate memory for map\n");
        exit(1);
    }
    map->width = width;
    map->height = height;
    for (int i = 0; i < width * height; i++) {
        map->cells[i] = WALL;
    }
    return map;
}

/**
 * function drawMap
 * @brief Draw the map
 * 
 * @param renderer 
 * @param map 
 * @return void
 */
void drawMap(SDL_Renderer *renderer, Map *map, TTF_Font *font) {
    SDL_Color textColor = { 255, 255, 255, 255 }; // White
    SDL_Color bgColor = { 0, 0, 0, 255 }; // Black

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };

            // Define the background color based on the cell type
            switch (map->cells[y * map->width + x]) {
                case WALL:
                    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
                    break;
                case PATH:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    break;
                case START:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for the starting point
                    break;
                case END:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for the ending point
                    break;
            }
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderDrawRect(renderer, &rect);

            // Display the coordinates on the first row and column
            if (x == 0 || y == 0) {
                char coords[4];
                if (x == 0 && y != 0) {
                    sprintf(coords, "%d", y);
                } else if (y == 0 && x != 0) {
                    sprintf(coords, "%d", x);
                }

                if ((x == 0 && y != 0) || (y == 0 && x != 0)) {
                    SDL_Surface *surface = TTF_RenderText_Solid(font, coords, textColor);
                    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
                    int text_width = surface->w;
                    int text_height = surface->h;
                    SDL_FreeSurface(surface);

                    SDL_Rect textRect = { x * CELL_SIZE + (CELL_SIZE - text_width) / 2, y * CELL_SIZE + (CELL_SIZE - text_height) / 2, text_width, text_height };
                    SDL_RenderCopy(renderer, texture, NULL, &textRect);
                    SDL_DestroyTexture(texture);
                }
            }
        }
    }
}

/**
 * function carvePathFrom
 * @brief Carve a path from a given point
 * 
 * @param x 
 * @param y 
 * @param map
 * @return void
 */
void carvePathFrom(int x, int y, Map *map) {
    int directions[] = { 0, 1, 2, 3 };
    // Mélange des directions
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4;
        int temp = directions[i];
        directions[i] = directions[r];
        directions[r] = temp;
    }

    for (int i = 0; i < 4; i++) {
        int dx = 0, dy = 0;
        switch (directions[i]) {
            case 0: dy = -1; break;
            case 1: dy = 1; break;
            case 2: dx = -1; break;
            case 3: dx = 1; break;
        }

        int nx = x + 2 * dx;
        int ny = y + 2 * dy;

        if (nx >= 0 && nx < map->width && ny >= 0 && ny < map->height && map->cells[ny * map->width + nx] == WALL) {
            map->cells[(ny - dy) * map->width + (nx - dx)] = PATH;
            map->cells[ny * map->width + nx] = PATH;
            carvePathFrom(nx, ny, map);
        }
    }
}

/**
 * function generateMap
 * @brief Generate a map with multiple paths (and on some paths no walls)
 * 
 * @param map 
 * @return void
 */
void generateMap(Map *map) {
    // Initialize the map with walls
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            map->cells[y * map->width + x] = WALL;
        }
    }

    // Create multiple paths by carving out a grid-like pattern
    for (int y = 1; y < map->height; y += 2) {
        for (int x = 1; x < map->width; x += 2) {
            map->cells[y * map->width + x] = PATH;
            if (x + 1 < map->width) {
                map->cells[y * map->width + x + 1] = PATH; // carve right
            }
            if (y + 1 < map->height) {
                map->cells[(y + 1) * map->width + x] = PATH; // carve down
            }
        }
    }

    // Create open areas without walls
    for (int y = 3; y < map->height; y += 4) {
        for (int x = 3; x < map->width; x += 4) {
            map->cells[y * map->width + x] = PATH;
            if (x + 1 < map->width) {
                map->cells[y * map->width + x + 1] = PATH; // open right
            }
            if (y + 1 < map->height) {
                map->cells[(y + 1) * map->width + x] = PATH; // open down
            }
            if (x - 1 > 0) {
                map->cells[y * map->width + x - 1] = PATH; // open left
            }
            if (y - 1 > 0) {
                map->cells[(y - 1) * map->width + x] = PATH; // open up
            }
        }
    }
}

/**
 * function setSpecialPoint
 * @brief Set the Special Point object
 * 
 * @param map 
 * @param x 
 * @param y 
 * @param state 
 * @return void
 */
void setSpecialPoint(Map *map, int x, int y, int state) {
    if (x >= 0 && x < map->width && y >= 0 && y < map->height) {
        map->cells[y * map->width + x] = state;
    }
}

/**
 * function isPath
 * @brief Check if a cell is a path
 * 
 * @param map 
 * @param x 
 * @param y 
 * @return int
 */
int isPath(Map *map, int x, int y) {
    if (x >= 0 && x < map->width && y >= 0 && y < map->height) {
        return map->cells[y * map->width + x] == PATH;
    }
    return 0;
}

/**
 * function placePoint
 * @brief Place a point on the map
 * 
 * @param map 
 * @param x 
 * @param y 
 * @param state 
 * @return void
 */
void placePoint(Map *map, int x, int y, int state) {
    int count = 0;
    if (count == 5) {
        printf("Cannot place point at (%d, %d): Too many points\n", x, y);
        return;
    }
    if (isPath(map, x, y)) {
        setSpecialPoint(map, x, y, state);
    } else {
        printf("Cannot place point at (%d, %d): Not a path\n", x, y);
    }
}

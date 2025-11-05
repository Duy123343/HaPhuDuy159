#include <SDL.h>
#include <SDL_image.h> 
#include <SDL_mixer.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int PLAYER_SIZE = 30;
const int ASTEROID_SIZE = 25;
const int PLAYER_SPEED = 6;
const int SPAWN_INTERVAL_MS = 1000;
const std::string HIGHSCORE_FILE = "highscore.txt";

const char* PLAYER_IMG_PATH = "player.png";
const char* ASTEROID_IMG_PATH = "asteroid.png"; 
const char* COLLISION_SOUND_PATH = "collision.wav";
const char* MUSIC_PATH = "space.ogg";

struct Player {
    SDL_Rect rect;
    float x;
    int speed;
};

struct Asteroid {
    SDL_Rect rect;
    float y_pos;
    float speed;
};

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
bool gRunning = true;

SDL_Texture* gPlayerTexture = NULL;
SDL_Texture* gAsteroidTexture = NULL;

Mix_Chunk* gCollisionSound = NULL;
Mix_Music* gMusic = NULL;

Player gPlayer;
std::vector<Asteroid> gAsteroids;
int gScore = 0;
int gHighScore = 0;
float gGameSpeedMultiplier = 1.0f;
Uint32 gLastSpawnTime = 0;

bool init();
bool loadMedia(); 
void close();
void handleEvents();
void update(float deltaTime);
void render();
void resetGame();
void spawnAsteroid();
bool checkCollision(const SDL_Rect& a, const SDL_Rect& b);
void loadHighScore();
void saveHighScore();

SDL_Texture* loadTexture(const char* path) {
    SDL_Texture* newTexture = NULL;

    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Khong the tai anh tu %s! SDL_image Error: %s\n", path, IMG_GetError());
    }
    else {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Khong the tao texture tu %s! SDL Error: %s\n", path, SDL_GetError());
        }

        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void loadHighScore() {
    std::ifstream file(HIGHSCORE_FILE);
    if (file.is_open()) {
        file >> gHighScore;
        if (file.fail()) {
            gHighScore = 0;
        }
        file.close();
    }
    else {
        gHighScore = 0;
    }
}

void saveHighScore() {
    if (gScore > gHighScore) {
        gHighScore = gScore;
        std::ofstream file(HIGHSCORE_FILE);
        if (file.is_open()) {
            file << gHighScore;
            file.close();
        }
        else {
            printf("WARNING: Khong the ghi diem cao vao file %s\n", HIGHSCORE_FILE.c_str());
        }
    }
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    int mixFlags = MIX_INIT_OGG | MIX_INIT_MP3;
    if ((Mix_Init(mixFlags) & mixFlags) != mixFlags) {
        printf("SDL_mixer could not initialize OGG/MP3 support! Mix Error: %s\n", Mix_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }

    gWindow = SDL_CreateWindow("Asteroid Dodge Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    srand((unsigned int)time(0));
    loadHighScore();
    resetGame();
    return true;
}

bool loadMedia() {
    bool success = true;

    gPlayerTexture = loadTexture(PLAYER_IMG_PATH);
    if (gPlayerTexture == NULL) {
        success = false;
    }

    gAsteroidTexture = loadTexture(ASTEROID_IMG_PATH);
    if (gAsteroidTexture == NULL) {
        success = false;
    }

    gCollisionSound = Mix_LoadWAV(COLLISION_SOUND_PATH);
    if (gCollisionSound == NULL) {
        printf("Khong the tai am thanh %s! SDL_mixer Error: %s\n", COLLISION_SOUND_PATH, Mix_GetError());
    }

    gMusic = Mix_LoadMUS(MUSIC_PATH);
    if (gMusic == NULL) {
        printf("WARNING: Khong the tai nhac nen %s! SDL_mixer Error: %s\n", MUSIC_PATH, Mix_GetError());
    }

    if (gMusic != NULL) {
        if (Mix_PlayMusic(gMusic, -1) == -1) {
            printf("WARNING: Khong the phat nhac nen! Mix_PlayMusic Error: %s\n", Mix_GetError());
        }
    }

    return success;
}

void resetGame() {
    saveHighScore();

    gPlayer.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
    gPlayer.rect = { (int)gPlayer.x, SCREEN_HEIGHT - 50, PLAYER_SIZE, PLAYER_SIZE };
    gPlayer.speed = PLAYER_SPEED;

    gAsteroids.clear();
    gScore = 0;
    gGameSpeedMultiplier = 1.0f;
    gLastSpawnTime = SDL_GetTicks();
}

void close() {
    saveHighScore();

    Mix_FreeMusic(gMusic);
    gMusic = NULL;

    Mix_FreeChunk(gCollisionSound);
    gCollisionSound = NULL;
    Mix_Quit();

    SDL_DestroyTexture(gPlayerTexture);
    SDL_DestroyTexture(gAsteroidTexture);
    gPlayerTexture = NULL;
    gAsteroidTexture = NULL;

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
    gWindow = NULL;

    IMG_Quit(); 
    SDL_Quit();
}

void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            gRunning = false;
        }
    }

    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

    if (currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_A]) {
        gPlayer.x -= gPlayer.speed;
    }
    if (currentKeyStates[SDL_SCANCODE_RIGHT] || currentKeyStates[SDL_SCANCODE_D]) {
        gPlayer.x += gPlayer.speed;
    }

    if (gPlayer.x < 0) gPlayer.x = 0;
    if (gPlayer.x > SCREEN_WIDTH - PLAYER_SIZE) gPlayer.x = SCREEN_WIDTH - PLAYER_SIZE;

    gPlayer.rect.x = (int)gPlayer.x;
}

void spawnAsteroid() {
    Uint32 currentTime = SDL_GetTicks();
    float currentSpawnInterval = SPAWN_INTERVAL_MS / gGameSpeedMultiplier;

    if (currentTime > gLastSpawnTime + currentSpawnInterval) {
        Asteroid newAsteroid;

        int randX = rand() % (SCREEN_WIDTH - ASTEROID_SIZE);

        newAsteroid.y_pos = -ASTEROID_SIZE;
        newAsteroid.rect = { randX, (int)newAsteroid.y_pos, ASTEROID_SIZE, ASTEROID_SIZE };

        newAsteroid.speed = (float)(rand() % 3 + 2);

        gAsteroids.push_back(newAsteroid);
        gLastSpawnTime = currentTime;
    }
}

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return (a.x < b.x + b.w &&
        a.x + a.w > b.x &&
        a.y < b.y + b.h &&
        a.y + a.h > b.y);
}

void update(float deltaTime) {
    gGameSpeedMultiplier = 1.0f + (float)gScore / 25.0f * 0.2f;

    spawnAsteroid();

    for (size_t i = 0; i < gAsteroids.size(); ++i) {
        gAsteroids[i].y_pos += gAsteroids[i].speed * gGameSpeedMultiplier * deltaTime * 60.0f;
        gAsteroids[i].rect.y = (int)gAsteroids[i].y_pos;

        if (checkCollision(gPlayer.rect, gAsteroids[i].rect)) { 
            if (gCollisionSound != NULL) {
                Mix_PlayChannel(-1, gCollisionSound, 0);
            }
            if (Mix_PlayingMusic() != 0) {
                Mix_HaltMusic();
            }
            printf("GAME OVER! Diem so cuoi cung: %d\n", gScore); 
            saveHighScore();
            resetGame();
            return;
        }

        if (gAsteroids[i].y_pos > SCREEN_HEIGHT) {
            gAsteroids.erase(gAsteroids.begin() + i);
            gScore++;
            i--; 
        }
    }
}

void render() {
    SDL_SetRenderDrawColor(gRenderer, 15, 20, 35, 255);
    SDL_RenderClear(gRenderer);

    if (gPlayerTexture) {
        SDL_RenderCopy(gRenderer, gPlayerTexture, NULL, &gPlayer.rect);
    }
    else {
        SDL_SetRenderDrawColor(gRenderer, 0, 200, 50, 255);
        SDL_RenderFillRect(gRenderer, &gPlayer.rect);
    }

    if (gAsteroidTexture) {
        for (const auto& asteroid : gAsteroids) {
            SDL_RenderCopy(gRenderer, gAsteroidTexture, NULL, &asteroid.rect);
        }
    }
    else {
        SDL_SetRenderDrawColor(gRenderer, 100, 100, 100, 255);
        for (const auto& asteroid : gAsteroids) {
            SDL_RenderFillRect(gRenderer, &asteroid.rect);
        }
    }

    std::string title = "Asteroid Dodge | Diem: " + std::to_string(gScore) +
        " | Diem Cao: " + std::to_string(gHighScore) +
        " | Toc do: " + std::to_string(gGameSpeedMultiplier).substr(0, 4);
    SDL_SetWindowTitle(gWindow, title.c_str());

    SDL_RenderPresent(gRenderer);
}

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Failed to initialize game!\n");
        return 1;
    }

    if (!loadMedia()) {
        printf("Failed to load media (images)! Vui long kiem tra file player.png va asteroid.png\n");
    }

    Uint32 lastTime = SDL_GetTicks();

    while (gRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        handleEvents();

        update(deltaTime);

        render();
    }

    close();
    return 0;
}


// SDL_Project.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <string> 
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_TTF.h"
#include "SDL_mixer.h"
#include "InputModule.h"
#include "GSLogoState.h"
#include "InputModule.h"


using namespace std;

///////// Definicion de estructuras /////////////
#include "StructsDef.h"
///////// Definicion de estructuras /////////////


///////// Variables y Constantes Globales /////////////
const int WIDTH = 1280;
const int HEIGHT = 720;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_bool isGameRunning = SDL_TRUE;
float time_multiplier = 1.0f;
InputState gameInputState;
SpriteAssets spritesAssets;
TextAssets textAssets;
BgmAssets musicAssets;
GameStages gameStages;

ResourceManager resourceManager;

///////// Variables y Constantes Globales /////////////
Ball* ball = new Ball(WIDTH / 2, HEIGHT / 2);
Pallet* pallet0 = new Pallet(10, HEIGHT / 2);
Pallet* pallet1 = new Pallet(WIDTH - 30, HEIGHT / 2);

///////// Funciones de inicializacion y destruccion /////////////
void initEngine()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    // fuck the music
    //Mix_OpenAudio(441/0, MIX_DEFAULT_FORMAT, 2, 1024);

    TTF_Init();

    int initted = Mix_Init(MIX_INIT_MP3);
    if ( (initted & MIX_INIT_MP3) != MIX_INIT_MP3) {
        cout << "Mix_Init: Failed to init required ogg and mod support!" << endl;
        cout << "Mix_Init: " << Mix_GetError() << endl;
        // handle error
    }

    window = SDL_CreateWindow("PONG", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Initializing Resource Manager
    resourceManager.spritesAssets = &spritesAssets;
    resourceManager.textAssets = &textAssets;
    resourceManager.musicAssets = &musicAssets;
    resourceManager.gameStages = &gameStages;
    resourceManager.inputState = &gameInputState;

    // Starting Game stage
    GameStage logoGameStage;
    logoGameStage.game_stageID = GS_GAMEPLAY;
    logoGameStage.stage_name = "gameplay";

    gameStages.push(logoGameStage);
}

void destroyEngine() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
}

///////// Funciones de carga y liberacion de recursos /////////////



void loadIMG(string path, int x, int y, int w, int h) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;

    Sprite sprite;
    sprite.dest = dest;
    sprite.texture = texture;
    spritesAssets.push_back(sprite);
}

void loadAssets() {

    int palletW = 20;
    int palletH = 100;

    loadIMG("assets/img/pallet.png", WIDTH - 30, HEIGHT / 2, palletW, palletH);
    loadIMG("assets/img/pallet.png", 10, HEIGHT / 2, palletW, palletH);
    loadIMG("assets/img/ball.png", WIDTH / 2, HEIGHT / 2, 20, 20);

    if (ball->direction == STOP) ball->ranBallDir();

    // Cargo el texto...
    string fontfilePath = "assets/fonts/arial.ttf";

    TTF_Font* Sans = TTF_OpenFont(fontfilePath.c_str(), 24);

    SDL_Color White = { 255, 255, 255 };

    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "0", White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
    
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture
    
    SDL_Rect Message_rect; //create a rect
    Message_rect.w = 40; // controls the width of the rect
    Message_rect.h = 40; // controls the height of the rect
    Message_rect.x = (WIDTH / 2) - 100;  //controls the rect's x coordinate 
    Message_rect.y = 100; // controls the rect's y coordinte
    
    Text player0score;
    player0score.font = Sans;
    player0score.color = White;
    player0score.surface = surfaceMessage;
    player0score.texture = Message;
    player0score.dest = Message_rect;
    
    textAssets.push_back(player0score);

    SDL_Rect Message_rect1; //create a rect
    Message_rect1.w = 40; // controls the width of the rect
    Message_rect1.h = 40; // controls the height of the rect
    Message_rect1.x = (WIDTH / 2) + 100;  //controls the rect's x coordinate 
    Message_rect1.y = 100; // controls the rect's y coordinte

    Text player1score;
    player1score.font = Sans;
    player1score.color = White;
    player1score.surface = surfaceMessage;
    player1score.texture = Message;
    player1score.dest = Message_rect1;

    textAssets.push_back(player1score);

    // Cargo Sonidos y BGM
    //string soundFilePath = "assets/bgm/littleidea.mp3";
    //Mix_Music* music;
    //music = Mix_LoadMUS(soundFilePath.c_str());
    //
    //Bgm bgm01;
    //bgm01.music = music;
    //
    //musicAssets.push_back(bgm01);

}

void unloadAssets() {
    for (int i = 0; i < spritesAssets.size(); i++) {
        SDL_DestroyTexture(spritesAssets[i].texture);
    }

    for (int i = 0; i < textAssets.size(); i++) {
        SDL_FreeSurface(textAssets[i].surface);
        SDL_DestroyTexture(textAssets[i].texture);
    }

    for (int i = 0; i < musicAssets.size(); i++) {
        Mix_FreeMusic(musicAssets[i].music);
    }
}

///////// Funciones de carga y liberacion de recursos /////////////

///////// Funciones de actualizacion y pintado /////////////



void inputUpdate() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        
        switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                pallet1->moveUp();
                spritesAssets[0].dest.y = pallet1->y;
                break;
            case SDLK_DOWN:
                pallet1->moveDown();
                spritesAssets[0].dest.y = pallet1->y;
                break;
            case SDLK_w:
                pallet0->moveUp();
                spritesAssets[1].dest.y = pallet0->y;
                break;
            case SDLK_s:
                pallet0->moveDown();
                spritesAssets[1].dest.y = pallet0->y;
                break;
            default:
                break;
            }
            break;
        case SDL_KEYUP:
            onKeyUp(event.key.keysym.sym, gameInputState);
            break;
        case SDL_MOUSEBUTTONDOWN:
            break;
        case SDL_MOUSEBUTTONUP:
            break;
        case SDL_QUIT:
            isGameRunning = SDL_FALSE;
            break;
        default:
            break;
        }
    }
}

// Para ser usado en distintos contadores..
float timer = 1.0f * 1000; // 1000 ms

void updateGame(float deltaTime) {
    const float BLINK_SPEED = 5.0f;

    timer -= BLINK_SPEED * deltaTime;

    // Small state machine using stack collection
    switch (gameStages.top().game_stageID ) {
    case GS_LOGO:
        GSLogoStateUpdate(deltaTime, resourceManager);
        break;
    case GS_MAIN_MENU:
        break;
    case GS_GAMEPLAY:
        break;
    case GS_INVALID:
    default:
        break;
    }

}

void render()
{
    // Limpio la pantalla 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Pinto todos los sprites...
    for (int i = 0; i < spritesAssets.size(); i++) {
        if (spritesAssets[i].isVisible) {
            ball->move();
            spritesAssets[2].dest.x = ball->x;
            spritesAssets[2].dest.y = ball->y;
            SDL_RenderCopy(renderer, spritesAssets[i].texture, NULL, &spritesAssets[i].dest);
        }
    }

    // Pinto todos los textos...
    for (int i = 0; i < textAssets.size(); i++) {
        if (textAssets[i].isVisible) {
            SDL_RenderCopy(renderer, textAssets[i].texture, NULL, &textAssets[i].dest);
        }
    }
    
    // Presento la imagen en pantalla
    SDL_RenderPresent(renderer);
}

///////// Funciones de actualizacion y pintado /////////////

///////// Funcione principal y GameLoop 



void replaceInTextAssetForScore(int pos, int score) {
    string tmp = to_string(score);
    char const* scoreChar = tmp.c_str();

    string fontfilePath = "assets/fonts/arial.ttf";

    TTF_Font* Sans = TTF_OpenFont(fontfilePath.c_str(), 24);

    SDL_Color White = { 255, 255, 255 };

    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, scoreChar, White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture

    SDL_Rect Message_rect; //create a rect
    Message_rect.w = 40; // controls the width of the rect
    Message_rect.h = 40; // controls the height of the rect
    if (pos == 1) Message_rect.x = (WIDTH / 2) + 100;  //controls the rect's x coordinate 
    if (pos == 0) Message_rect.x = (WIDTH / 2) - 100;  //controls the rect's x coordinate 
    Message_rect.y = 100; // controls the rect's y coordinte

    Text scoreText;
    scoreText.font = Sans;
    scoreText.color = White;
    scoreText.surface = surfaceMessage;
    scoreText.texture = Message;
    scoreText.dest = Message_rect;

    textAssets[pos] = scoreText;
}

int p0Scores = 0;
int p1Scores = 0;

void monitorBall() {

    if (ball->y <= 5) ball->changeDir(ball->direction == UPRIGHT ? DOWNRIGHT : DOWNLEFT);
    if (ball->y >= HEIGHT - 5) ball->changeDir(ball->direction == DOWNRIGHT ? UPRIGHT : UPLEFT);
    
    // player0   player1
    if (ball->x >= WIDTH - 1) {
        // player 0 scores
        p0Scores++;
        replaceInTextAssetForScore(0, p0Scores);

        ball->resetBall();
    } 
    
    if (ball->x <= 0) {
        // player 1 scores
        p1Scores++;
        replaceInTextAssetForScore(1, p1Scores);
        ball->resetBall();
    }

    if (ball->x >= pallet1->x-20) {
        int pallet1y = pallet1->y;
        if (ball->y >= pallet1->y && ball->y <= (pallet1->y + 100)) ball->ranBalDirLeft();
    }

    if (ball->x <= pallet0->x + 20) {
        int pallet0y = pallet0->y;
        if (ball->y >= pallet0->y && ball->y <= (pallet0->y + 100)) ball->ranBalDirRight();
    }
}

int main(int argc, char* argv[])
{
    srand((unsigned)time(NULL));

    initEngine();

    // Cargo Assets
    loadAssets();

    //Mix_PlayMusic(musicAssets[0].music, -1);

    Uint64 currentTime = SDL_GetTicks64();

    while (isGameRunning) {

        

        Uint64 previousTime = currentTime;

        currentTime = SDL_GetTicks64();

        Uint64 deltaTime = currentTime - previousTime;

        inputUpdate();
        monitorBall();

        updateGame(deltaTime * time_multiplier);

        render();
    }

    // Detendo la musica 
    Mix_HaltMusic();

    // Descargo Assets
    unloadAssets(); 
    destroyEngine();
    return 0;
}


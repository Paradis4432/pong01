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
SpriteAssets menuAssets;

TextAssets textAssets;
TextAssets textMenuAssets;

BgmAssets musicAssets;

ResourceManager resourceManager;
GameStages gameStages;


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
    if ((initted & MIX_INIT_MP3) != MIX_INIT_MP3) {
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
    resourceManager.inputState = &gameInputState;

    GameStage mainMenu;
    mainMenu.game_stageID = GS_MAIN_MENU;
    gameStages.push(mainMenu);
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


    // lamba callback to gen messages 
    auto genMessage = [](
        TTF_Font* font,
        SDL_Color color,
        string message,
        int w,
        int h,
        int x,
        int y,
        boolean autoDefineTextSize) {

            const char* messageChar = message.c_str();
            SDL_Surface* messageSettings = TTF_RenderText_Solid(font, messageChar, color);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(::renderer, messageSettings);

            SDL_Rect messageRect; //create a rect
            if (autoDefineTextSize) {
                SDL_Rect destR = { x, y, 0, 0 };
                TTF_SizeText(font, messageChar, &destR.w, &destR.h);

                messageRect.w = destR.w; // controls the width of the rect
                messageRect.h = destR.h; // controls the height of the rect
            }
            else {
                messageRect.w = w; // controls the width of the rect
                messageRect.h = h; // controls the height of the rect
            }
            
            messageRect.x = x;  //controls the rect's x coordinate 
            messageRect.y = y; // controls the rect's y coordinte

            Text messageFinalSettings;
            messageFinalSettings.font = font;
            messageFinalSettings.color = color;
            messageFinalSettings.surface = messageSettings;
            messageFinalSettings.texture = texture;
            messageFinalSettings.dest = messageRect;

            return messageFinalSettings;
    };

    //textMenuAssets.push_back(mainMenuMessage);
    textMenuAssets.push_back(genMessage(Sans, White, "PONG", 0, 0, 500, 100, true));
    textMenuAssets.push_back(genMessage(Sans, White, "Arrows to move up and down", 0, 0, 100, 200, true));
    textMenuAssets.push_back(genMessage(Sans, White, "' u ' to start", 0, 0, 100, 270, true));
    textMenuAssets.push_back(genMessage(Sans, White, "' i ' for infinite mode, currently: disabled", 0, 0, 100, 340, true));



    textAssets.push_back(genMessage(Sans, White, "0", 40, 40, (WIDTH / 2) - 100, 100, false));
    textAssets.push_back(genMessage(Sans, White, "0", 40, 40, (WIDTH / 2) + 60, 100, false));

    int x = 0; 
    for (int i = 0; i < 36; i++) {
        x = x + 20;
        textAssets.push_back(genMessage(Sans, White, "|", 1, 12, WIDTH / 2, x, false));
    }

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
            case SDLK_u:
                if (gameStages.top().game_stageID == GS_MAIN_MENU) {
                    cout << "test\n";
                    GameStage gameplay;
                    gameplay.game_stageID = GS_GAMEPLAY;
                    gameStages.push(gameplay);
                }

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
    /*
    switch (gameStages.top().game_stageID ) {
    case GS_LOGO:
        GSLogoStateUpdate(deltaTime, resourceManager);
        break;
    case GS_MAIN_MENU:
        break;
    case GS_GAMEPLAY:
        break;
    case GS_INVALID:
        break;
    default:
        break;
    }
    */
}


void render()
{
    // Limpio la pantalla 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    switch (gameStages.top().game_stageID)
    {
    case GS_MAIN_MENU:
        for (int i = 0; i < textMenuAssets.size(); i++) {
            if (textMenuAssets[i].isVisible) {
                SDL_RenderCopy(renderer, textMenuAssets[i].texture, NULL, &textMenuAssets[i].dest);
            }
        }
        break;
    case GS_GAMEPLAY:
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
        break;
    default:
        break;
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

    switch (ball->direction) {
    case LEFT:
    case DOWNLEFT:
    case UPLEFT:
        if (ball->x < WIDTH / 2) {
            if (ball->y < pallet0->y + 50) pallet0->moveUp();
            if (ball->y > pallet0->y + 50) pallet0->moveDown();
            spritesAssets[1].dest.y = pallet0->y;
        }
    default:
        break;
    }


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

    if (ball->x >= pallet1->x - 20) {
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

    while (true) {
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


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
TextAssets gameOverTexts;

BgmAssets musicAssets;

ResourceManager resourceManager;
GameStages gameStages;


///////// Variables y Constantes Globales /////////////
Ball* ball = new Ball(WIDTH / 2, HEIGHT / 2);
Pallet* pallet0 = new Pallet(10, HEIGHT / 2);
Pallet* pallet1 = new Pallet(WIDTH - 30, HEIGHT / 2);

int p0Scores = 0;
int p1Scores = 0;
int seconds = 0;

bool infMode = false;
bool pvp = false;



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

// lamba callback to gen messages 
Text genMessage(
    string message,
    int w,
    int h,
    int x,
    int y,
    boolean autoDefineTextSize) {
        string fontfilePath = "assets/fonts/arial.ttf";
        TTF_Font* Sans = TTF_OpenFont(fontfilePath.c_str(), 24);
        SDL_Color White = { 255, 255, 255 };

        const char* messageChar = message.c_str();
        SDL_Surface* messageSettings = TTF_RenderText_Solid(Sans, messageChar, White);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(::renderer, messageSettings);

        SDL_Rect messageRect; //create a rect
        if (autoDefineTextSize) {
            SDL_Rect destR = { x, y, 0, 0 };
            TTF_SizeText(Sans, messageChar, &destR.w, &destR.h);

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
        messageFinalSettings.font = Sans;
        messageFinalSettings.color = White;
        messageFinalSettings.surface = messageSettings;
        messageFinalSettings.texture = texture;
        messageFinalSettings.dest = messageRect;

        return messageFinalSettings;
};

void loadAssets() {

    int palletW = 20;
    int palletH = 100;

    auto loadIMG = [](string path, int x, int y, int w, int h) {
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
    };

    loadIMG("assets/img/pallet.png", WIDTH - 30, HEIGHT / 2, palletW, palletH);
    loadIMG("assets/img/pallet.png", 10, HEIGHT / 2, palletW, palletH);
    loadIMG("assets/img/ball.png", WIDTH / 2, HEIGHT / 2, 20, 20);

    if (ball->direction == STOP) ball->ranBallDir();

    //textMenuAssets.push_back(mainMenuMessage);
    textMenuAssets.push_back(genMessage("PONG", 0, 0, 500, 100, true));
    textMenuAssets.push_back(genMessage("Arrows to move up and down for right side player", 0, 0, 100, 200, true));
    textMenuAssets.push_back(genMessage("W, S to move up and down for left side player", 0, 0, 100, 250, true));
    textMenuAssets.push_back(genMessage("' u ' to start", 0, 0, 100, 350, true));
    Text infDisabled = genMessage("' i ' to toggle infinite mode, currently: disabled", 0, 0, 100, 400, true);
    Text infEnabled = genMessage("' i ' to toggle infinite mode, currently: enabled", 0, 0, 100, 400, true);
    infDisabled.isVisible = true;
    infEnabled.isVisible = false;
    textMenuAssets.push_back(infDisabled);
    textMenuAssets.push_back(infEnabled);

    Text cpuDisabled = genMessage("' m ' to toggle cpu movement on left side, currently: enabled", 0, 0, 100, 450, true);
    Text cpuEnabled = genMessage("' m ' to toggle cpu movement on left side, currently: disabled", 0, 0, 100, 450, true);
    cpuDisabled.isVisible = true;
    cpuEnabled.isVisible = false;
    textMenuAssets.push_back(cpuDisabled);
    textMenuAssets.push_back(cpuEnabled);

    textAssets.push_back(genMessage("0", 40, 40, (WIDTH / 2) - 100, 100, false));
    textAssets.push_back(genMessage("0", 40, 40, (WIDTH / 2) + 60, 100, false));
    Text scorep0 = genMessage("SCORE", 0, 0, (WIDTH / 2) - 300, 450, true);
    scorep0.isVisible = false;
    Text scorep1 = genMessage("SCORE", 0, 0, (WIDTH / 2) + 180, 450, true);
    scorep1.isVisible = false;
    textAssets.push_back(scorep0);
    textAssets.push_back(scorep1);
    if(!infMode) textAssets.push_back(genMessage("0", 0, 0, 150, 20, true));
    if (!infMode) textAssets.push_back(genMessage("Timer: ", 0, 0, 80, 18, true));
    

    int x = 0; 
    for (int i = 0; i < 36; i++) {
        x = x + 20;
        textAssets.push_back(genMessage("|", 1, 12, WIDTH / 2, x, false));
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


void replaceInTextAssetForScore(int pos, int score) {

    string fontfilePath = "assets/fonts/arial.ttf";

    TTF_Font* Sans = TTF_OpenFont(fontfilePath.c_str(), 24);

    SDL_Color White = { 255, 255, 255 };

    string tmp = to_string(score);
    char const* scoreChar = tmp.c_str();

    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, scoreChar, White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture

    SDL_Rect Message_rect; //create a rect
    Message_rect.w = 40; // controls the width of the rect
    Message_rect.h = 40; // controls the height of the rect
    Message_rect.x = pos == 1 ? (WIDTH / 2) + 100 : (WIDTH / 2) - 100;
    Message_rect.y = 100; // controls the rect's y coordinte

    Text scoreText;
    scoreText.font = Sans;
    scoreText.color = White;
    scoreText.surface = surfaceMessage;
    scoreText.texture = Message;
    scoreText.dest = Message_rect;

    textAssets[pos] = scoreText;
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
        for (int i = 0; i < spritesAssets.size(); i++) {
            if (spritesAssets[i].isVisible) {
                ball->move();
                spritesAssets[2].dest.x = ball->x;
                spritesAssets[2].dest.y = ball->y;
                SDL_RenderCopy(renderer, spritesAssets[i].texture, NULL, &spritesAssets[i].dest);
            }
        }

        for (int i = 0; i < textAssets.size(); i++) {
            if (textAssets[i].isVisible) {
                SDL_RenderCopy(renderer, textAssets[i].texture, NULL, &textAssets[i].dest);
            }
        }
        break;
    case GS_GAMEOVER:
        if (true) {
            string winner = p0Scores == p1Scores ? "tie" : p0Scores > p1Scores ? "player 0" : "player 1";

            gameOverTexts.push_back(genMessage("Winner: " + winner, 0, 0, (WIDTH / 2) - 100, 100, true));
            gameOverTexts.push_back(genMessage("' r ' to go back to the menu", 0, 0, 100, 200, true));

            int pos = gameOverTexts.size();
            SDL_RenderCopy(renderer, gameOverTexts[pos - 1].texture, NULL, &gameOverTexts[pos - 1].dest);
            SDL_RenderCopy(renderer, gameOverTexts[pos - 2].texture, NULL, &gameOverTexts[pos - 2].dest);
        };

        break;
    default:
        break;
    }

    // Presento la imagen en pantalla
    SDL_RenderPresent(renderer);
}

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
                    GameStage gameplay;
                    gameplay.game_stageID = GS_GAMEPLAY;
                    gameStages.push(gameplay);

                    p0Scores = 0;
                    p1Scores = 0;
                }
                break;
            case SDLK_i:
                if (!(gameStages.top().game_stageID == GS_MAIN_MENU)) continue;

                textMenuAssets[4].isVisible = !textMenuAssets[4].isVisible;
                textMenuAssets[5].isVisible = !textMenuAssets[5].isVisible;
                infMode = !infMode;
                break;
            case SDLK_m:
                if (!(gameStages.top().game_stageID == GS_MAIN_MENU)) continue;

                textMenuAssets[6].isVisible = !textMenuAssets[6].isVisible;
                textMenuAssets[7].isVisible = !textMenuAssets[7].isVisible;
                pvp = !pvp;
                break;

            case SDLK_r:
                if (gameStages.top().game_stageID == GS_GAMEOVER) {
                    GameStage mainMenu;
                    mainMenu.game_stageID = GS_MAIN_MENU;
                    gameStages.push(mainMenu);


                }
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

///////// Funciones de actualizacion y pintado /////////////

///////// Funcione principal y GameLoop 

void replaceInTextAssetForTimer(int seconds) {

    string fontfilePath = "assets/fonts/arial.ttf";

    TTF_Font* Sans = TTF_OpenFont(fontfilePath.c_str(), 24);

    SDL_Color White = { 255, 255, 255 };

    string tmp = to_string(seconds);
    char const* secondsChar = tmp.c_str();
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, secondsChar, White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture

    SDL_Rect Message_rect; //create a rect

    SDL_Rect destR = { textAssets[4].dest.x, textAssets[4].dest.y, 0, 0 };
    TTF_SizeText(Sans, secondsChar, &destR.w, &destR.h);

    Message_rect.w = destR.w;
    Message_rect.h = destR.h;
    Message_rect.x = textAssets[4].dest.x;
    Message_rect.y = textAssets[4].dest.y;

    Text finalText;
    finalText.font = Sans;
    finalText.color = White;
    finalText.surface = surfaceMessage;
    finalText.texture = Message;
    finalText.dest = Message_rect;

    textAssets[4] = finalText;
}

void monitorBall() {

    if (ball->y <= 5) ball->changeDir(ball->direction == UPRIGHT ? DOWNRIGHT : DOWNLEFT);
    if (ball->y >= HEIGHT - 5) ball->changeDir(ball->direction == DOWNRIGHT ? UPRIGHT : UPLEFT);

    switch (ball->direction) {
    case LEFT:
    case DOWNLEFT:
    case UPLEFT:
        if (ball->x < WIDTH / 2 && !pvp) {
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

        textAssets[2].isVisible = true;
        Sleep(1000);
        ball->resetBall();
        textAssets[2].isVisible = false;

    }

    if (ball->x <= 0) {
        // player 1 scores
        p1Scores++;
        replaceInTextAssetForScore(1, p1Scores);
        textAssets[3].isVisible = true;
        Sleep(1000);
        ball->resetBall();
        textAssets[3].isVisible = false;
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
    //Uint64 currentTime = SDL_GetTicks64();
    unsigned int lastTime = 0, currentTime;

    while (true) {
        //Uint64 previousTime = currentTime;
        //currentTime = SDL_GetTicks64();
        //Uint64 deltaTime = currentTime - previousTime;

        inputUpdate();
        monitorBall();

        if (gameStages.top().game_stageID == GS_GAMEPLAY && !infMode) {
            currentTime = SDL_GetTicks();
            if (currentTime > lastTime + 1000) {
                lastTime = currentTime;
                seconds++;
                if (seconds == 30) {

                    pallet1->resetPos();
                    spritesAssets[0].dest.y = pallet1->y;
                    spritesAssets[0].dest.x = pallet1->x;

                    pallet0->resetPos();
                    spritesAssets[1].dest.y = pallet0->y;
                    spritesAssets[1].dest.x = pallet0->x;

                    ball->resetBall();
                    spritesAssets[2].dest.y = ball->y;
                    spritesAssets[2].dest.x = ball->x;

                    render();

                    seconds = 0;


                    replaceInTextAssetForScore(0, 0);
                    replaceInTextAssetForScore(1, 0);

                    GameStage finalMenu;
                    finalMenu.game_stageID = GS_GAMEOVER;
                    gameStages.push(finalMenu);

                }
                replaceInTextAssetForTimer(seconds);
            }
        }
        //updateGame(deltaTime * time_multiplier);
        

        render();
    }

    // Detendo la musica 
    Mix_HaltMusic();

    // Descargo Assets
    unloadAssets();
    destroyEngine();
    return 0;
}


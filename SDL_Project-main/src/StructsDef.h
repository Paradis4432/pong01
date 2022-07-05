#pragma once

using namespace std;
#include <fstream>
#include <vector>
#include <stack>
#include "SDL.h"
#include "SDL_TTF.h"
#include "SDL_mixer.h"
#include <windows.h>

typedef struct _InputState {
    bool up = false;
    bool down = false;
    bool right = false;
    bool left = false;
    bool fire = false;
}InputState;

typedef struct _Sprite {
    SDL_Texture* texture;
    SDL_Rect dest;
    bool isVisible = true;
}Sprite;

typedef struct _Text {
    TTF_Font* font;
    SDL_Color color;
    SDL_Surface* surface;
    SDL_Texture* texture;
    SDL_Rect dest;
    bool isVisible = true;
}Text;

typedef struct _Bgm {
    Mix_Music* music;
}Bgm;

enum GAME_STAGES
{
    GS_INVALID = -1,
    GS_LOGO = 0,
    GS_MAIN_MENU,
    GS_GAMEPLAY,
    GS_GAMEPAUSE,
    GS_GAMEOVER,
    GS_CREDITS
};

typedef struct _GameStage {
    string stage_name;
    int game_stageID = GS_INVALID;
    int level;
}GameStage;


typedef vector<Sprite> SpriteAssets;
typedef vector<Text> TextAssets;
typedef vector<Bgm> BgmAssets;
typedef stack<GameStage> GameStages;

typedef struct _ResourceManager {
    SpriteAssets* spritesAssets;
    TextAssets* textAssets;
    BgmAssets* musicAssets;
    GameStages* gameStages;
    InputState* inputState;
}ResourceManager;


class Pallet {
public:
    int x, y, iniX, iniY;

    Pallet(int x, int y) {
        this->iniX = x;
        this->iniY = y;
        this->x = x;
        this->y = y;
    }

    void resetPos() {
        x = iniX;
        y = iniY;
    }

    void moveUp() {
        y -= 20;
    }

    void moveDown() {
        y += 20;
    }
};


enum ballDir { STOP, LEFT, UPLEFT, DOWNLEFT, RIGHT, DOWNRIGHT, UPRIGHT};


class Ball {
public:
    int x, y, centerX, centerY;
    ballDir direction;

    Ball(int x, int y) {
        this->centerX = x;
        this->centerY = y;
        this->x = x;
        this->y = y;
        this->direction = STOP;
    }

    void resetBall() {
        x = centerX;
        y = centerY;
        ranBallDir();
    }

    void changeDir(ballDir d) {
        direction = d;
    }

    void ranBallDir() {
        direction = (ballDir)((rand() % 6) + 1);
        //direction = RIGHT;
    }
    
    void ranBalDirLeft() {
        direction = (ballDir)((rand() % 3) + 2);
    }

    void ranBalDirRight() {
        direction = (ballDir)((rand() % 3) + 5);
    }
    

    void move() {
        switch (direction) {
        case STOP:
            break;
        case LEFT:
            x--;
            x--;
            break;
        case RIGHT:
            x++;
            x++;
            break;
        case UPLEFT:
            x--;
            x--;
            y--;
            y--;
            break;
        case DOWNLEFT:
            x--;
            x--;
            y++;
            y++;
            break;
        case UPRIGHT:
            x++;
            x++;
            y--;
            y--;
            break;
        case DOWNRIGHT:
            x++;
            x++;
            y++;
            y++;
            break;
        default:
            break;
        }
    }
};
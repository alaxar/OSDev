#ifndef PONG_H
#define PONG_H

#include "stdint.h"
#include "stdbool.h"

// Menu defines
enum {
    MENU_MAIN,
    MENU_GAME,
    MENU_OPTIONS,
    MENU_CREDITS,
    MENU_EXIT,
    MENU_SOLOGAME,
    MENU_LANGAME,
    MENU_INTERNETGAME,
    _MENU_COUNT
};

// AppMode defines
#define APPMODE_MENU                   1
#define APPMODE_GAME                   2

// GameMode defines
#define GAMEMODE_NONE                  1
#define GAMEMODE_LOCALGAME_1P          2
#define GAMEMODE_LOCALGAME_2P          3
#define GAMEMODE_LANGAME_HOST          4
#define GAMEMODE_LANGAME_JOIN          5
#define GAMEMODE_INTERNETGAME_HOST     6
#define GAMEMODE_INTERNETGAME_JOIN     7

// Functions
static void DrawMainMenu(void);
static void DrawGameMenu(void);
static void DrawOptionsMenu(void);
static void DrawCreditsMenu(void);
static void DrawExitMenu(void);
static void DrawSoloGameMenu(void);
static void DrawLANGameMenu(void);
static void DrawInternetGameMenu(void);
static void DrawMenuStructure(void);
static void DrawMenuPoint(const char* name, bool selected, uint16_t y);
static void DrawMenuHeader(void);
static void DrawMenuContentBox(uint16_t addlines);
static void DrawMenuSlideIn(void);
static void DrawMenuSlideOut(void);

static void SwitchToMenu(uint8_t switchto);

static void Menu_SelectorUp(void);
static void Menu_SelectorDown(void);
static void Menu_Select(void);

static void DrawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
static void DrawText(const char* text, uint16_t x, uint16_t y);
static void SetPixel(uint16_t x, uint16_t y);
static void FlipIfNeeded(void);

static void Sound_Denied(void);
static void Sound_OK(void);
static void Sound_Select(void);
static void Sound_Error(void);
static void Sound_Goal(void);
static void Sound_GotIt(void);

static void NotImplementedError(void);
static void Error(const char* message, bool critical);

static void RenderApp(void);
static void ResetBall(void);
static void RunGame(void);
static void UpdateGame(void);
static void RenderGame(void);

static void GetGameControl(void);
static void WaitKey(void);


#endif

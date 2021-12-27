/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "pong.h"
#include "userlib.h"
#include "stdio.h"
#include "stdlib.h"


// Parameters:
static const uint8_t PLAYER_1_HEIGHT = 15;
static const uint8_t PLAYER_2_HEIGHT = 15;
static const uint8_t PLAYER_2_HEIGHT_SHORT = 1;
static const float   XSPEEDLOW = 1.5f;
static const float   XSPEEDHIGH = 2.5f;
static const float   YSPEEDLOW = -2.0f;
static const float   YSPEEDHIGH = 2.0f;
static const uint8_t MAXSCORE = 10;

// GFX:
static const bool DOUBLEBUFFERING = true;
static const uint16_t BLINKFREQ = 500;
static const uint8_t SLIDEDELAY = 5;
static const uint8_t COLUMNS = 80;
static const uint8_t LINES = 50;
static const uint8_t SCOREX = 10;
static const uint8_t SCOREY = 5;

// Default options:
static bool option_sound         = true;
static bool option_menuanimation = true;

// Variables
static uint8_t currentmenu = MENU_MAIN;
static uint8_t menu_selection[_MENU_COUNT] =  { 0, 0, 0, 0, 0, 0, 0, 0 };
static uint8_t menu_selectable[_MENU_COUNT] = { 4, 4, 3, 1, 2, 3, 3, 3 };

// Start in menu
static uint8_t appmode = APPMODE_MENU;
static uint8_t gamemode = GAMEMODE_NONE;

static int16_t player1y;       // Y Position on screen
static int16_t player2y;
static int16_t player1h;       // Height of player
static int16_t player2h;
static int16_t player1score;   // Player score
static int16_t player2score;
static int16_t player1game;    // Player games
static int16_t player2game;

static float   ballx;          // Ball X
static float   bally;          // Ball Y
static float   ballxspeed;     // Ball X-Speed
static float   ballyspeed;     // Ball Y-Speed
static bool    kickoff_to_the_right = true; // true (right), false (left)

static bool    exitapp = false; // If true, the application closes

int main(void)
{
    srand(getCurrentMilliseconds()); // seed

    // Adjust screen
    if (DOUBLEBUFFERING)
    {
        console_setProperties(CONSOLE_FULLSCREEN);
    }
    else
    {
        console_setProperties(CONSOLE_AUTOREFRESH|CONSOLE_FULLSCREEN);
    }

    setScrollField(0,50);

    // Enable Events
    event_enable(true);
    char buffer[1024];
    EVENT_t ev = event_poll(buffer, 1024, EVENT_NONE);

    // Play animation
    DrawMenuSlideIn();

    // Initial drawing
    DrawMainMenu();

    while(exitapp == false)
    {
        switch(ev)
        {
            case EVENT_NONE:
            {
                // Re-render App (overwrite any message)
                if (DOUBLEBUFFERING == false)
                    clearScreen(0x00);

                RenderApp();
                FlipIfNeeded();

                if (DOUBLEBUFFERING)
                    clearScreen(0x00);

                waitForEvent(20);
                break;
            }
            case EVENT_KEY_DOWN:
            {
                KEY_t* key = (void*)buffer;

                if(*key == KEY_ESC)
                {
                    switch(appmode)
                    {
                        case APPMODE_MENU:
                            switch (currentmenu)
                            {
                                case MENU_MAIN:
                                    SwitchToMenu(MENU_EXIT);
                                    break;
                                case MENU_CREDITS:
                                case MENU_OPTIONS:
                                case MENU_EXIT:
                                case MENU_GAME:
                                    SwitchToMenu(MENU_MAIN);
                                    break;
                                case MENU_SOLOGAME:
                                case MENU_LANGAME:
                                case MENU_INTERNETGAME:
                                    SwitchToMenu(MENU_GAME);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            Error("An unknown error occurred (1): %u",appmode);
                            break;
                    }
                }
                else if(*key == KEY_ARRU || *key == KEY_W || *key == KEY_P)
                {
                    switch(appmode)
                    {
                        case APPMODE_MENU:
                            Menu_SelectorUp();
                            break;
                        default:
                            Error("An unknown error occurred (3): %u",appmode);
                            break;
                    }
                }
                else if(*key == KEY_ARRD || *key == KEY_S || *key == KEY_L)
                {
                    switch(appmode)
                    {
                        case APPMODE_MENU:
                            Menu_SelectorDown();
                            break;
                        default:
                            Error("An unknown error occurred (4): %u",appmode);
                            break;
                    }
                }
                else if(*key == KEY_ENTER || *key == KEY_SPACE)
                {
                    switch(appmode)
                    {
                        case APPMODE_MENU:
                            Menu_Select();
                            break;
                        default:
                            Error("An unknown error occurred (5): %u",appmode);
                            break;
                    }
                }

                break;
            }
            default:
                break;
        }
        ev = event_poll(buffer, 1024, EVENT_NONE);
    }

    DrawMenuSlideOut();

    console_setProperties(CONSOLE_AUTOREFRESH|CONSOLE_AUTOSCROLL|CONSOLE_FULLSCREEN);
    return (0);
}

// Play Game

static void RenderGame(void)
{
    // Draw border around the playfield
    textColor(0x88); DrawRect(0, 0, COLUMNS,   LINES  );
    textColor(0x00); DrawRect(1, 1, COLUMNS-1, LINES-1);

    // Draw line in the middle
    textColor(0x11); DrawRect((COLUMNS/2),1,(COLUMNS/2)+1,LINES-1);

    // Draw players
    textColor(0xAA); DrawRect(3,         player1y, 5,         player1y+player1h);
    textColor(0xEE); DrawRect(COLUMNS-5, player2y, COLUMNS-3, player2y+player2h);

    // Draw ball
    iSetCursor(ballx-1,bally-1);
    textColor(0x0C); putchar('/');   // x-1
    textColor(0xCC); putchar('X');   // x
    textColor(0x0C); putchar('\\');  // x+1

    iSetCursor(ballx-1,bally);
    textColor(0xCC); putchar('X');   // x-1
                     putchar('X');   // x
                     putchar('X');   // x+1

    iSetCursor(ballx-1,bally+1);
    textColor(0x0C); putchar('\\');  // x-1
    textColor(0xCC); putchar('X');   // x
    textColor(0x0C); putchar('/');   // x+1

    // Draw Score and Game
    if (player1score == MAXSCORE)
    {
        player1game++;
    }

    if (player2score == MAXSCORE)
    {
        player2game++;
    }

    iSetCursor(SCOREX,SCOREY);
    textColor(0x07); printf("score: ");
    textColor(0x0D); printf("%u", player1score);
    textColor(0x07); printf(" : ");
    textColor(0x0D); printf("%u", player2score);

    iSetCursor(SCOREX,SCOREY+1);
    textColor(0x07); printf("game:  ");
    textColor(0x0D); printf("%u", player1game);
    textColor(0x07); printf(" : ");
    textColor(0x0D); printf("%u", player2game);

    if ( (player1score == MAXSCORE) || (player2score == MAXSCORE) )
    {
        player1score = player2score = 0;
        beep(220,100);
        beep(440,100);
        beep(880,100);

      #ifdef AI
        player1h -= 2*player1game;
        if (player1h < 5)
        {
            player1h = 5;
        }
      #endif
    }
}

static void UpdateGame(void)
{
    ballx = (ballx + ballxspeed);
    bally = (bally + ballyspeed);

    //////////////////////
    // collision / goal //
    //////////////////////

    if((bally-2) < 0) // collision upper wall
    {
        ballyspeed = -ballyspeed;
        bally = 2;
    }
    else if((bally+2) > LINES) // collision lower wall
    {
        ballyspeed = -ballyspeed;
        bally = (LINES - 3);
    }

    if( ballx > 5 && ballx < 7 && bally >= player1y && bally <= player1y + player1h ) // player1 got it
    {
        Sound_GotIt();

        if (ballxspeed < 0)
        {
            ballxspeed = -ballxspeed;
            ballx++;
            ballyspeed += random(-0.2,0.2);
        }
    }

    if( ballx > COLUMNS - 7 && ballx < COLUMNS - 5 && bally >= player2y && bally <= player2y + player2h ) // player2 got it
    {
        Sound_GotIt();

        if (ballxspeed > 0)
        {
            ballxspeed = -ballxspeed;
            ballx--;
            ballyspeed += random(-0.2,0.2);
        }
    }

    if( ballx < 2 && ballxspeed < 0 ) // goal left
    {
        player2score++;
        kickoff_to_the_right = true;
        Sound_Goal();
        ResetBall();
        sleep(500);
    }
    else if( ballx > COLUMNS - 2 && ballxspeed > 0 ) // goal right
    {
        player1score++;
        kickoff_to_the_right = false;
        Sound_Goal();
        ResetBall();
        sleep(500);
    }

    GetGameControl(); // get input from player
}

static void ResetBall(void)
{
    ballx = (COLUMNS / 2);
    bally = (LINES / 2);

    do
    {
        ballxspeed = random(XSPEEDLOW, XSPEEDHIGH);
        ballyspeed = random(YSPEEDLOW, YSPEEDHIGH);
    }
    while( (ballyspeed > -0.5 && ballyspeed < 0.5) || ballxspeed == 0);

    if((kickoff_to_the_right == true && ballxspeed<0) || (kickoff_to_the_right == false && ballxspeed>0))
    {
        ballxspeed = -(ballxspeed);
    }
}

static void RunGame(void)
{
    switch (gamemode)
    {
        case GAMEMODE_NONE:
            Error("An unknown error occurred.",true);
            break;
        case GAMEMODE_LOCALGAME_1P:
            player2h = PLAYER_2_HEIGHT_SHORT;
            break;
        case GAMEMODE_LOCALGAME_2P:
            player2h = PLAYER_2_HEIGHT;
            break;
        default:
            Error("An unknown error occurred (10).",true);
            break;
    }

    char buffer[1024];
    EVENT_t ev = event_poll(buffer, 1024, EVENT_NONE);

    bool exitgame = false;

    player1h = PLAYER_1_HEIGHT;
    player1y = (LINES / 2) - (player1h / 2);

    player2y = (LINES / 2) - (player2h / 2);

    player1score = player1game = 0;
    player2score = player2game = 0;

    ResetBall();

    while(exitgame == false)
    {
        switch(ev)
        {
            case EVENT_NONE:
                // Re-render App (overwrite any message)
                if (DOUBLEBUFFERING == false)
                    clearScreen(0x00);

                UpdateGame();
                RenderGame();
                FlipIfNeeded();

                if (DOUBLEBUFFERING)
                    clearScreen(0x00);

                waitForEvent(1);
                break;

            case EVENT_KEY_DOWN:
            {
                KEY_t* key = (void*)buffer;

                if (*key == KEY_ESC)
                {
                    exitgame = true;
                }
                break;
            }
            default:
                break;
        }
        ev = event_poll(buffer, 1024, EVENT_NONE);
    }
}




static void RenderApp(void)
{
    switch(appmode)
    {
        case APPMODE_MENU:
            switch (currentmenu)
            {
                case MENU_MAIN:
                    DrawMainMenu();
                    break;
                case MENU_EXIT:
                    DrawExitMenu();
                    break;
                case MENU_CREDITS:
                    DrawCreditsMenu();
                    break;
                case MENU_GAME:
                    DrawGameMenu();
                    break;
                case MENU_OPTIONS:
                    DrawOptionsMenu();
                    break;
                case MENU_SOLOGAME:
                    DrawSoloGameMenu();
                    break;
                case MENU_LANGAME:
                    DrawLANGameMenu();
                    break;
                case MENU_INTERNETGAME:
                    DrawInternetGameMenu();
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

// Menu

static void Menu_SelectorUp(void)
{
    if (currentmenu >= _MENU_COUNT)
    {
        Sound_Error();
        return;
    }
    if (menu_selection[currentmenu] == 0)
        menu_selection[currentmenu] = menu_selectable[currentmenu] - 1;
    else
        menu_selection[currentmenu]--;
    Sound_Select();
}

static void Menu_SelectorDown(void)
{
    if (currentmenu >= _MENU_COUNT)
    {
        Sound_Error();
        return;
    }
    if (menu_selection[currentmenu] == menu_selectable[currentmenu] - 1)
        menu_selection[currentmenu] = 0;
    else
        menu_selection[currentmenu]++;
    Sound_Select();
}

static void Menu_Select(void)
{
    switch (currentmenu)
    {
        case MENU_MAIN:
            switch (menu_selection[MENU_MAIN])
            {
                case 0: // New game
                    Sound_OK();
                    SwitchToMenu(MENU_GAME);
                    break;
                case 1: // Options
                    Sound_OK();
                    SwitchToMenu(MENU_OPTIONS);
                    break;
                case 2:
                    Sound_OK();
                    SwitchToMenu(MENU_CREDITS);
                    break;
                case 3: // Exit
                    Sound_OK();
                    SwitchToMenu(MENU_EXIT);
                    break;
                default:
                    break;
            }
            break;
        case MENU_EXIT:
            switch (menu_selection[MENU_EXIT])
            {
                case 0: // Yes
                    Sound_OK();
                    exitapp = true;
                    break;
                case 1: // No
                    Sound_Denied();
                    SwitchToMenu(MENU_MAIN);
                    break;
                default:
                    break;
            }
            break;
        case MENU_CREDITS:
            switch (menu_selection[MENU_CREDITS])
            {
                case 0:
                    Sound_OK();
                    SwitchToMenu(MENU_MAIN);
                    break;
                default:
                    break;
            }
            break;
        case MENU_GAME:
            switch (menu_selection[MENU_GAME])
            {
                case 0: // Local
                    Sound_OK();
                    SwitchToMenu(MENU_SOLOGAME);
                    break;
                case 1: // LAN
                    Sound_OK();
                    SwitchToMenu(MENU_LANGAME);
                    break;
                case 2: // Internet
                    Sound_OK();
                    SwitchToMenu(MENU_INTERNETGAME);
                    break;
                case 3: // Main menu
                    Sound_OK();
                    SwitchToMenu(MENU_MAIN);
                    break;
                default:
                    break;
            }
            break;
        case MENU_OPTIONS:
            switch (menu_selection[MENU_OPTIONS])
            {
                case 0: // Sound
                    option_sound = !option_sound;
                    Sound_OK();
                    break;
                case 1: // Menu animations
                    option_menuanimation = !option_menuanimation;
                    Sound_OK();
                    break;
                case 2:
                    Sound_OK();
                    SwitchToMenu(MENU_MAIN);
                    break;
                default:
                    break;
            }
            break;
        case MENU_SOLOGAME:
            switch (menu_selection[MENU_SOLOGAME])
            {
                case 0: // One player
                    gamemode=GAMEMODE_LOCALGAME_1P;
                    appmode=APPMODE_GAME;
                    RunGame();
                    appmode=APPMODE_MENU;
                    break;
                case 1: // Two players
                    gamemode=GAMEMODE_LOCALGAME_2P;
                    appmode=APPMODE_GAME;
                    RunGame();
                    appmode=APPMODE_MENU;
                    break;
                case 2: // GameMenu
                    Sound_OK();
                    SwitchToMenu(MENU_GAME);
                    break;
                default:
                    break;
            }
            break;
        case MENU_LANGAME:
            switch (menu_selection[MENU_LANGAME])
            {
                case 0: // Join game
                    NotImplementedError();
                    break;
                case 1: // Host game
                    NotImplementedError();
                    break;
                case 2: // GameMenu
                    Sound_OK();
                    SwitchToMenu(MENU_GAME);
                    break;
                default:
                    break;
            }
            break;
        case MENU_INTERNETGAME:
            switch (menu_selection[MENU_INTERNETGAME])
            {
                case 0: // Join game
                    NotImplementedError();
                    break;
                case 1: // Host game
                    NotImplementedError();
                    break;
                case 2: // GameMenu
                    Sound_OK();
                    SwitchToMenu(MENU_GAME);
                    break;
            }
            break;
        default:
        {
            // This should not happen!
            char errormsg[100];
            sprintf(errormsg, "An unknwown error occurred (2): %u", currentmenu);
            Error(errormsg, true);
            break;
        }
    }
}

static void DrawMainMenu(void)
{
    DrawMenuStructure();

    textColor(0x0D);
    DrawText("Main menu",21,24);

    DrawMenuPoint("New game", menu_selection[MENU_MAIN] == 0, 29);
    DrawMenuPoint("Options", menu_selection[MENU_MAIN] == 1, 31);
    DrawMenuPoint("Credits", menu_selection[MENU_MAIN] == 2, 33);
    DrawMenuPoint("Exit", menu_selection[MENU_MAIN] == 3, 37);
}

static void DrawYesNo(bool string_condition, bool color_condition)
{
    if (color_condition)
        textColor(0x0F);
    else
        textColor(0x08);
    putchar('[');

    if (string_condition)
    {
        if (color_condition)
            textColor(0x0A);
        else
            textColor(0x02);
        printf("YES");
    }
    else
    {
        if (color_condition)
            textColor(0x0C);
        else
            textColor(0x04);
        printf("NO");
    }

    if (color_condition)
        textColor(0x0F);
    else
        textColor(0x08);
    putchar(']');
}

static void DrawOptionsMenu(void)
{
    DrawMenuStructure();

    textColor(0x0D);
    DrawText("Options:",21,24);


    DrawMenuPoint("Sound", menu_selection[MENU_OPTIONS] == 0, 29);

    iSetCursor(38,29);
    DrawYesNo(option_sound, menu_selection[MENU_OPTIONS] == 0);

    // option_menuanimation
    DrawMenuPoint("Menu animations", menu_selection[MENU_OPTIONS] == 1, 31);

    iSetCursor(48, 31);
    DrawYesNo(option_menuanimation, menu_selection[MENU_OPTIONS] == 1);

    DrawMenuPoint("Done", menu_selection[MENU_OPTIONS] == 2, 38);

}

static void DrawSoloGameMenu(void)
{
    DrawMenuStructure();

    textColor(0x0D);   DrawText("Local game type:",21,24);

    DrawMenuPoint("1 player", menu_selection[MENU_SOLOGAME] == 0, 29);
    DrawMenuPoint("2 players", menu_selection[MENU_SOLOGAME] == 1, 31);
    DrawMenuPoint("Return to game menu", menu_selection[MENU_SOLOGAME] == 2, 38);
}

static void DrawLANGameMenu(void)
{
    DrawMenuStructure();

    textColor(0x0D);    DrawText("LAN game:",21,24);

    DrawMenuPoint("Join game", menu_selection[MENU_LANGAME] == 0, 29);
    DrawMenuPoint("Host game", menu_selection[MENU_LANGAME] == 1, 31);
    DrawMenuPoint("Return to game menu", menu_selection[MENU_LANGAME] == 2, 38);
}

static void DrawInternetGameMenu(void)
{
    DrawMenuStructure();

    textColor(0x0D);    DrawText("Internet game:",21,24);

    DrawMenuPoint("Join game", menu_selection[MENU_INTERNETGAME] == 0, 29);
    DrawMenuPoint("Host game", menu_selection[MENU_INTERNETGAME] == 1, 31);
    DrawMenuPoint("Return to game menu", menu_selection[MENU_INTERNETGAME] == 2, 38);
}

static void DrawCreditsMenu(void)
{
    DrawMenuStructure();

    textColor(0x0D);
    DrawText("Credits:",21,24);

    textColor(0x09);
    DrawText("Copyright (c) 2011-2015",21,30);
    DrawText("The PrettyOS Project" ,21,32);
    DrawText("All rights reserved"  ,21,34);

    DrawMenuPoint("Done", menu_selection[MENU_CREDITS] == 0, 38);

}

static void DrawExitMenu(void)
{
    DrawMenuStructure();

    textColor(0x0D);   DrawText("Do you really want to exit?",21,24);

    DrawMenuPoint("Yes", menu_selection[MENU_EXIT] == 0, 30);
    DrawMenuPoint("No", menu_selection[MENU_EXIT] == 1, 32);
}

static void DrawGameMenu(void)
{
    DrawMenuStructure();

    textColor(0x0D);    DrawText("New game / Gametype:",21,24);

    DrawMenuPoint("Local game", menu_selection[MENU_GAME] == 0, 29);
    DrawMenuPoint("Local network (LAN) game", menu_selection[MENU_GAME] == 1, 31);
    DrawMenuPoint("Internet game", menu_selection[MENU_GAME] == 2, 33);
    DrawMenuPoint("Return to main menu", menu_selection[MENU_GAME] == 3, 36);
}

static void DrawMenuStructure(void)
{
    DrawMenuHeader();
    DrawMenuContentBox(0);
}

static void DrawMenuPoint(const char* name, bool selected, uint16_t y)
{
    iSetCursor(26, y);

    if (selected)
        textColor(0x0E);
    else
        textColor(0x08);
    printf("[ ");

    if (selected && getCurrentMilliseconds() % BLINKFREQ < BLINKFREQ/2)
        putchar('X');
    else
        putchar(' ');
    printf(" ]");

    if (selected)
        textColor(0x0F);
    else
        textColor(0x08);
    printf(" %s", name);
}

static void DrawMenuSlideIn(void)
{
    if(option_menuanimation)
    {
        for(uint16_t y = 26; y>0; y--)
        {
            clearScreen(0x00);
            DrawMenuContentBox(y);
            DrawMenuHeader();
            FlipIfNeeded();
            sleep(SLIDEDELAY);
        }
    }
    clearScreen(0x00);
}

static void DrawMenuSlideOut(void)
{
    if(option_menuanimation)
    {
        for(uint16_t y = 0; y<25; y--)
        {
            clearScreen(0x00);
            DrawMenuContentBox(y);
            DrawMenuHeader();
            FlipIfNeeded();
            sleep(SLIDEDELAY);
        }
    }
    clearScreen(0x00);
}

static void SwitchToMenu(uint8_t switchto)
{
    DrawMenuSlideOut();
    if(switchto == MENU_EXIT)
    {
        menu_selection[MENU_EXIT] = 0;
    }

    if(switchto == MENU_MAIN)
    {
        menu_selection[MENU_MAIN] = 0;
    }

    if (switchto == MENU_GAME && menu_selection[MENU_GAME] == (menu_selectable[MENU_GAME] - 1))
    {
        menu_selection[MENU_GAME] = 0;
    }

    currentmenu = switchto;
    DrawMenuSlideIn();
}

static void DrawMenuContentBox(uint16_t addlines)
{
    iSetCursor(0,22);
    for(uint16_t i = 0; i<addlines; i++)
    {
        putchar('\n');
    }

    textColor(0x0F);

    bool do_exit = false;
    for(uint16_t line = 0; line<25;line++)
    {
        if((line+addlines+22)>49)
        {
            do_exit = true;
        }

        textColor(0x0F);

        if(do_exit == false)
        {
            switch (line)
            {
                case 0:
                    printf("                   ");
                    printf("#####################################\n");
                    break;
                case 1:
                    printf("                   ");
                    putchar('#');
                    printf("\t\t\t\t        ");
                    printf("#\n");
                    break;
                case 2:
                    printf("                   ");
                    putchar('#');
                    printf("\t\t\t   \t         ");
                    printf("#\n");
                    break;
                case 4:
                    printf("                   ");
                    putchar('#');
                    textColor(0x07);
                    printf("======================================");
                    textColor(0x0F);
                    printf("#\n");
                    break;
                case 17:
                    printf("                    ");
                    putchar('#');
                    printf("\t\t\t\t          ");
                    printf("#\n");
                    break;
                case 18:
                    printf("                     ");
                    putchar('#');
                    printf("\t\t\t\t          ");
                    printf("#\n");
                    break;
                case 19:
                    printf("                      #####################################\n");
                    break;
                case 20:
                case 21:
                case 23:
                    putchar('\n');
                    break;
                case 22:
                    textColor(0x0E);
                    printf("  Use the arrow keys to navigate and Enter or the Spacebar to select.\n");
                    textColor(0x0F);
                    break;
                case 24:
                    textColor(0x09);
                    printf("  If you can't use the arrow keys, you can use W and S or P and L instead.\n");
                    break;
                default:
                    printf("                   ");
                    putchar('#');
                    printf("\t\t\t\t          ");
                    printf("#\n");
                    break;
            }
        }
    }
}


// GFX

static void DrawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    for(uint16_t lx = x1; lx<x2; lx++)
    {
        for(uint16_t ly = y1; ly<y2; ly++)
        {
            SetPixel(lx,ly);
        }
    }
}

static void DrawText(const char* text, uint16_t x, uint16_t y)
{
    iSetCursor(x,y);
    puts(text);
}

static void SetPixel(uint16_t x, uint16_t y)
{
    iSetCursor(x,y);
    putchar('x');
}

static void FlipIfNeeded(void)
{
    if (DOUBLEBUFFERING)
    {
        refreshScreen();
    }
}

// Input

static void GetGameControl(void)
{
    switch(gamemode)
    {
        case GAMEMODE_LOCALGAME_1P:
            // Player 1

            if(keyPressed(KEY_ARRU) || keyPressed(KEY_P) || (keyPressed(KEY_W)))
            {
                player1y = (player1y - 3);
            }

            if(keyPressed(KEY_ARRD) || keyPressed(KEY_L) || keyPressed(KEY_S))
            {
                player1y = (player1y + 3);
            }

            if(player1y < 2)
            {
                player1y = 2;
            }

            if((player1y+player1h) > (LINES - 2))
            {
                player1y = ((LINES - 2) - player1h);
            }

            // Player 2 (AI)

            player2y = bally - player2h/2; // AI Strategy

            if(player2y < 2)
            {
                player2y = 2;
            }

            if((player2y+player2h) > (LINES - 2))
            {
                player2y = ((LINES - 2) - player2h);
            }

            break;

        case GAMEMODE_LOCALGAME_2P:
            // Player 1

            if(keyPressed(KEY_W))
            {
                player1y = (player1y - 3);
            }

            if(keyPressed(KEY_S))
            {
                player1y = (player1y + 3);
            }

            if(player1y < 2)
            {
                player1y = 2;
            }

            if((player1y+player1h) > (LINES - 2))
            {
                player1y = ((LINES - 2) - player1h);
            }

            // Player 2

            if(keyPressed(KEY_P))
            {
                player2y = (player2y - 3);
            }

            if(keyPressed(KEY_L))
            {
                player2y = (player2y + 3);
            }

            if(player2y < 2)
            {
                player2y = 2;
            }

            if((player2y+player2h) > (LINES - 2))
            {
                player2y = ((LINES - 2) - player2h);
            }
            break;
    }
}

static void WaitKey(void)
{
    char buffer[1024];
    EVENT_t ev = event_poll(buffer, 1024, EVENT_NONE);

    while(true)
    {
        switch(ev)
        {
            case EVENT_NONE:
                waitForEvent(10);
                break;
            case EVENT_KEY_DOWN:
                return;
            default:
                break;
        }

        ev = event_poll(buffer, 1024, EVENT_NONE);
    }
}

// Error

static void NotImplementedError(void)
{
    Error("This feature is not (yet) implemented.",false);
}

static void Error(const char* message, bool critical)
{
    if(appmode==APPMODE_MENU)
    {
        DrawMenuSlideOut();
    }

    clearScreen(0x00);
    iSetCursor(10,7);
    textColor(0x0F);
    puts(message);

    iSetCursor(10,9);

    if(critical == false)
    {
        printf("Press any key to continue...");
    }
    else
    {
        printf("Press any key to exit...");
        exitapp = true;
    }

    printf("\n\n\n\n\n\n\n");
    printf("                              ");
    textColor(0x04);
    printf("#####         #####\n");
    printf("                              ");
    putchar('#');
    textColor(0x0C);
    printf("#####       #####");
    textColor(0x04);
    printf("#\n");
    printf("                               ");
    textColor(0x04);
    putchar('#');
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#     #");
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#\n");
    printf("                                ");
    textColor(0x0C);
    printf("#####     #####\n");
    printf("                                ");
    textColor(0x04);
    putchar('#');
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#   #");
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#\n");
    printf("                                 #");
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("# #");
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#\n");
    printf("                                  ");
    textColor(0x0C);
    printf("##### #####\n");
    printf("                                  ");
    textColor(0x04);
    putchar('#');
    textColor(0x0C);
    printf("#########");
    textColor(0x04);
    printf("#\n");
    printf("                                   #");
    textColor(0x0C);
    printf("#######");
    textColor(0x04);
    printf("#\n");
    printf("                                    ");
    textColor(0x0C);
    printf("#######\n");
    printf("                                    ");
    textColor(0x04);
    putchar('#');
    textColor(0x0C);
    printf("#####");
    textColor(0x04);
    printf("#\n");
    printf("                                    #");
    textColor(0x0C);
    printf("#####");
    textColor(0x04);
    printf("#\n");
    printf("                                   #");
    textColor(0x0C);
    printf("#######\n");
    printf("                                   #########\n");
    printf("                                  ");
    textColor(0x04);
    putchar('#');
    textColor(0x0C);
    printf("#########");
    textColor(0x04);
    printf("#\n");
    printf("                                 #");
    textColor(0x0C);
    printf("##### #####\n");
    printf("                                 #####");
    textColor(0x04);
    printf("# #");
    textColor(0x0C);
    printf("#####\n");
    printf("                                ");
    textColor(0x04);
    putchar('#');
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#   #");
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#\n");
    printf("                               #");
    textColor(0x0C);
    printf("#####     #####\n");
    printf("                               #####");
    textColor(0x04);
    printf("#     #");
    textColor(0x0C);
    printf("#####\n");
    printf("                              ");
    textColor(0x04);
    putchar('#');
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#       #");
    textColor(0x0C);
    printf("####");
    textColor(0x04);
    printf("#\n");
    printf("                              ");
    textColor(0x0C);
    printf("#####         #####\n");
    putchar('\n');

    FlipIfNeeded();
    Sound_Error();
    WaitKey();
    clearScreen(0x00);

    if(appmode==APPMODE_MENU)
    {
        DrawMenuSlideIn();
    }

    iSetCursor(0,0);
    textColor(0x0F);
}

static void DrawMenuHeader(void)
{
    iSetCursor(0,2);

    textColor(0x0E);
    putchar('\n');
    printf("    Created: %s, %s from %s",__DATE__,__TIME__,__FILE__);
    printf("\n\n");
    printf("    ");textColor(0x99);
    printf("#######");textColor(0x09);
    printf("                               ");textColor(0x99);
    printf("#######\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("              ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("          ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("              ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("          ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    putchar(' ');textColor(0x99);
    printf("####");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("####");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##########");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    putchar(' ');textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("####");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("#####");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("#####\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    putchar(' ');textColor(0x99);
    printf("###");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    putchar(' ');textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    putchar(' ');textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    putchar(' ');textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("#######");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("######");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("#######");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("       ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("      ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("       ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("       ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("      ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("####");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("       ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("       ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("####");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("       ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("       ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("####");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("        ");textColor(0x99);
    printf("####");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("   ");textColor(0x99);
    printf("#####\n");textColor(0x09);
    printf("                                    ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("                                 ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("                                   ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("                              ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("  ");textColor(0x99);
    printf("##\n");textColor(0x09);
    printf("                                  ");textColor(0x99);
    printf("##");textColor(0x09);
    printf("                                ");textColor(0x99);
    printf("####\n\n");textColor(0x0E);
}

// Sound

static void Sound_Denied(void)
{
    if(option_sound)
    {
        beep(600,150);
        beep(400,150);
    }
}

static void Sound_OK(void)
{
    if(option_sound)
    {
        beep(800,100);
        beep(1000,100);
    }
}

static void Sound_Select(void)
{
    if(option_sound)
    {
        beep(1000,50);
    }
}

static void Sound_Error(void)
{
    if(option_sound)
    {
        Sound_Denied();
        sleep(500);
    }
}

void Sound_Goal(void)
{
    if(option_sound)
    {
        beep(200,50);
    }
}

static void Sound_GotIt(void)
{
    if(option_sound)
    {
        beep(800,20);
    }
}

/*
* Copyright (c) 2011-2015 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

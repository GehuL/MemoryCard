#include <stdarg.h>
#include <time.h>
#include "Board.h"
#include "raygui.h"
#include "Utils.h"

#define RSCP "rsc/" // ressource path
#define TEXTURES_P "rsc/textures/"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#ifdef _DEBUG
#define debugP print_debug
#else
#define debugP(f, ...)
#endif // DEBUG

#define TUTO "Clique gauche = Voir carte\nClique droit = Recommencer\n\nH = Cacher les cartes trouvees\nN = Mode difficile\n\nEchap = Quitter"

#define FONT_SIZE 30
#define FONT "C:\\Windows\\Fonts\\crete-round.ttf"

#define MAX_TEXTURE 32

#define CENTER_IN_SCREEN(width, height) \
    (Vector2) { GetScreenWidth() / 2 - width / 2, GetScreenHeight() / 2 - height / 2 }

#define SAVE_NAME "save"

#define REVEALED_DURATION 5 // Seconds
#define CLICKED_DURATION 3

//------------------------------------------------------------------------------------
// Variables de jeu
//------------------------------------------------------------------------------------

int found;  // Nombre de cartes trouv�es.
int essais; // Tentatives pour trouver une paire.

bool hardMode;
bool hideFoundCard;

double temps; // Temps qui s'écoule pendant la partie en seconde

Card cards[MAX_CARDS]; // Liste des cartes sur le plateau
Card *pair[2];         // Pointeur sur les cartes cliqu�es

Texture2D backCard;    // Texture du dos de carte
Texture2D backGround;  // Arri�re plans
Texture2D victoryCard; // Carte de victoire
Texture2D dropShadow;  // Selection de carte

Sound soundFound;
Sound soundHide;
Sound soundShow;
Sound soundShuffle;
Sound soundVictory;

Font font;

GameState state;
double timer, click_time, clock_time; // Variables pour la gestion du temps

Shader blurShader;

#define QWERTY
#ifdef QWERTY
const int konamiCode[10] = {KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_B, KEY_Q}; // key Q car qwerty
#elif AZERTY
const int konamiCode[10] = {KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_B, KEY_A};
#endif
int indexKonami = 0;

//------------------------------------------------------------------------------------
// Variables du moteur
//------------------------------------------------------------------------------------

bool running, keepRatio;

int baseHeight, baseWidth;
int gameScreenHeight, gameScreenWidth;

float screenRatio; // Ratio width / height.
float widthRatio = 1, heightRatio = 1;

RenderTexture renderer, blurRenderer;

SaveData gSaveData = {0};
char best_score[256] = "?";

//------------------------------------------------------------------------------------
// Fontions
//------------------------------------------------------------------------------------
void revealAllCard(bool);
void drawCard(void);
void drawBoard(void);
void on_timer(void);

static void shuffle(void *array, size_t n, size_t size)
{
    char tmp[size];
    char *arr = array;
    size_t stride = size * sizeof(char);

    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; ++i)
        {
            size_t rnd = (size_t)rand();
            size_t j = i + rnd / (RAND_MAX / (n - i) + 1);

            memcpy(tmp, arr + j * stride, size);
            memcpy(arr + j * stride, arr + i * stride, size);
            memcpy(arr + i * stride, tmp, size);
        }
    }
}

void print_debug(const char *format, ...)
{
    va_list vl;
    va_start(vl, format);
    vprintf(format, vl);
    va_end(vl);
    putchar('\n');
}

void placeCard() // Positionne en x, y les cartes
{
    int originX = renderer.texture.width / 2 - (backCard.width + WIDTH_GAP) * 2; // On centre les cartes au mileu
    int originY = renderer.texture.height / 2 - (backCard.height + HEIGHT_GAP) * 2;

    for (int y = 0, i = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
        {
            cards[i].hitbox.x = cards[i].pos.x = originX + x * (cards[i].texture.width + WIDTH_GAP);
            cards[i].hitbox.y = cards[i].pos.y = originY + y * (cards[i].texture.height + HEIGHT_GAP);
            i++;
        }
}

void reset(void)
{
    for (int i = 0; i < MAX_CARDS; i++)
    {
        cards[i].revealed = true;
        cards[i].found = false;
        cards[i].index = i;
    }
    found = 0;
    essais = 0;
    temps = 0.;
    memset(pair, 0, 2 * sizeof(Card *));
    state = Revealed;
    clock_time = timer = GetTime();
}

void replay(void)
{
    shuffle(cards, MAX_CARDS, sizeof(Card));
    placeCard();
    reset();
    PlaySound(soundShuffle);
}

// Full path doit �tre moins grand que 65 char
Texture2D loadRessource(const char *path, const char *name)
{
    Texture2D texture = LoadTexture(TextFormat("%s%s", path, name));
    if (texture.id == 0)
    {
        board_unload();
        exit(EXIT_FAILURE);
    }

    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    return texture;
}

bool LoadSave(SaveData *save)
{
    unsigned int bytesRead = -1;
    unsigned char *data = LoadFileData(SAVE_NAME, &bytesRead);

    if (bytesRead == sizeof(SaveData))
    {
        memcpy(save, (const void *)data, sizeof(SaveData));
        return true;
    }
    return false;
}

void SaveScore(void)
{
    gSaveData.bestScore = essais;
    gSaveData.bestTime = temps;
    GetUserName(gSaveData.userName, sizeof(gSaveData.userName));
    SaveFileData(SAVE_NAME, (void *)&gSaveData, sizeof(SaveData));
}

void board_load(int width, int height, const char *title)
{
    srand(time(NULL));

    // SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    // SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(width, height, title);

    SetTargetFPS(60);

    Image image = LoadImage("Icon.png");
    SetWindowIcon(image);
    UnloadImage(image);

    InitAudioDevice();

    SetMasterVolume(0.8);

    baseWidth = gameScreenWidth = width;

    baseHeight = gameScreenHeight = height;

    screenRatio = (float)width / (float)height;

    font = LoadFont(FONT);

    soundFound = LoadSound("rsc/sounds/card_found.ogg");
    SetSoundVolume(soundFound, 0.5);

    soundShow = LoadSound("rsc/sounds/card_show.ogg");

    soundHide = LoadSound("rsc/sounds/card_hide.ogg");

    soundShuffle = LoadSound("rsc/sounds/card_shuffle.ogg");
    SetSoundVolume(soundShuffle, 0.5);

    soundVictory = LoadSound("rsc/sounds/card_victory.ogg");

    renderer = LoadRenderTexture(baseWidth, baseHeight);

    blurRenderer = LoadRenderTexture(baseWidth, baseHeight);

    blurShader = LoadShader(0, "rsc/shaders/blur.fs");

    // #ifndef DEBUG
    //     SetWindowMinSize(1080, 768);
    // #endif // DEBUG

    backCard = loadRessource(TEXTURES_P, "BackCard.png");

    // Proportionnel a la fenetre
    backCard.width = 10 * GetScreenWidth() / 100;
    backCard.height = 23 * GetScreenHeight() / 100;

    victoryCard = loadRessource(TEXTURES_P, "victory_card.png");

    victoryCard.width = backCard.width * 2;
    victoryCard.height = backCard.height * 2;

    backGround = loadRessource(TEXTURES_P, "Background.png");

    dropShadow = loadRessource(TEXTURES_P, "drop_shadow.png");

    dropShadow.width = backCard.width + 10;
    dropShadow.height = backCard.height + 10;

    backGround.width = GetScreenWidth();
    backGround.height = GetScreenHeight();

    static const char images[16][20] = {"AceClubs.png", "AceDiamonds.png", "AceHearts.png", "AceSpades.png",
                                        "JackClubs.png", "JackDiamonds.png", "JackHearts.png", "JackSpades.png",
                                        "KingClubs.png", "KingDiamonds.png", "KingHearts.png", "KingSpades.png",
                                        "QueenClubs.png", "QueenDiamonds.png", "QueenHearts.png", "QueenSpades.png"};
    for (int i = 0; i < MAX_CARDS / 2; i++)
    {
        cards[i].texture = loadRessource(TEXTURES_P, images[i]);

        cards[i].hitbox.height = cards[i].texture.height = backCard.height;
        cards[i].hitbox.width = cards[i].texture.width = backCard.width;
    }
    // Creating copys.
    for (int x = 0, i = MAX_CARDS / 2; i < MAX_CARDS; i++, x++)
        cards[i] = cards[x];

    hardMode = 0;

    hideFoundCard = 0;

    keepRatio = true;

    if (LoadSave(&gSaveData))
    {
        sprintf(best_score, "\nEssais: %d\nTemps: %d\npar %s", gSaveData.bestScore, gSaveData.bestTime, gSaveData.userName);
    }
    else
    {
        sprintf(best_score, "%s", "?");
        gSaveData.bestScore = 0xffffff;
    }
    state = Revealed;
}

void board_unload(void)
{
    UnloadRenderTexture(renderer);
    UnloadRenderTexture(blurRenderer);

    UnloadTexture(backCard);
    UnloadTexture(backGround);
    UnloadTexture(victoryCard);

    UnloadFont(font);

    UnloadShader(blurShader);

    UnloadSound(soundFound);
    UnloadSound(soundShuffle);
    UnloadSound(soundShow);
    UnloadSound(soundHide);
    UnloadSound(soundVictory);

    for (int i = 0; i < 16; i++)
        UnloadTexture(cards[i].texture);

    CloseAudioDevice();
    CloseWindow();
}

void hideClickedCard(void)
{
    if (pair[0] && pair[1])
    {
        pair[0]->revealed = false;
        pair[1]->revealed = false;

        pair[0] = NULL;
        pair[1] = NULL;

        PlaySound(soundHide);
    }
}

void drawCard(void)
{
    if (pair[0] != NULL)
    {
        DrawTexture(dropShadow, pair[0]->pos.x + (pair[0]->texture.width - dropShadow.width) / 2, pair[0]->pos.y + (pair[0]->texture.height - dropShadow.height) / 2, WHITE);

        if (pair[1] != NULL)
            DrawTexture(dropShadow, pair[1]->pos.x + (pair[1]->texture.width - dropShadow.width) / 2, pair[1]->pos.y + (pair[1]->texture.height - dropShadow.height) / 2, WHITE);
    }

    for (int i = 0; i < MAX_CARDS; i++)
    {
        if (cards[i].found)
        {
            if (!hideFoundCard)
                DrawTextureV(cards[i].texture, cards[i].pos, WHITE);
        }
        else if (cards[i].revealed)
        {
            DrawTextureV(cards[i].texture, cards[i].pos, WHITE);
        }
        else
            DrawTextureV(backCard, cards[i].pos, WHITE);
    }
}

void drawBoard(void)
{
    // ----------------------------------------- //
    // ------------ TEXTURE DRAWING ------------ //
    // ----------------------------------------- //
    BeginTextureMode(blurRenderer);

    DrawTexture(backGround, 0, 0, WHITE);
    drawCard();

    EndTextureMode();

    BeginTextureMode(renderer);
    if (state == Win)
    {
        BeginShaderMode(blurShader);
        DrawTextureRec(blurRenderer.texture, (Rectangle){0, 0, (float)blurRenderer.texture.width, (float)-blurRenderer.texture.height}, (Vector2){0, 0}, WHITE);
        EndShaderMode();
        DrawTextureV(victoryCard, CENTER_IN_SCREEN(victoryCard.width, victoryCard.height), WHITE);
    }
    else
        DrawTextureRec(blurRenderer.texture, (Rectangle){0, 0, (float)blurRenderer.texture.width, (float)-blurRenderer.texture.height}, (Vector2){0, 0}, WHITE);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(renderer.texture,
                   (Rectangle){0, 0, (float)renderer.texture.width, (float)-renderer.texture.height},
                   (Rectangle){0, 0, gameScreenWidth, gameScreenHeight},
                   (Vector2){0, 0}, 0, WHITE);

    // -------------------------------------- //
    // ------------ TEXT DRAWING ------------ //
    // -------------------------------------- //

    // INFO
    DrawTextEx(font, TextFormat("Essai%s: %d\nTemps: %d\n\nRecord: %s", essais > 1 ? "s" : "", essais, (int)temps, best_score), (Vector2){0, 10}, FONT_SIZE, 2, DARKPURPLE);

    // TUTO
    DrawTextEx(font, TUTO, (Vector2){gameScreenWidth - 265, 0}, FONT_SIZE - 10, 2, DARKPURPLE);

#ifdef _DEBUG
    DrawText(TextFormat("%d\nx: %d y: %d", GetFPS(), GetMouseX(), GetMouseY()), 0, GetScreenHeight() - (FONT_SIZE + 20), FONT_SIZE - 10, RED);
#endif

    EndDrawing();
}

void revealAllCard(bool reveal)
{
    for (int i = 0; i < MAX_CARDS; i++)
        cards[i].revealed = reveal;
}

void updatePlayingState(void)
{
    // Gestion des cartes cliqués
    if (pair[1] != NULL) // Deux cartes sont cliqués
    {
        if (GetTime() - click_time > CLICKED_DURATION)
        {
            click_time = GetTime();
            hideClickedCard();
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        for (int i = 0; i < MAX_CARDS; i++)
        {
            if (!cards[i].found && !cards[i].revealed) // Si n'est pas deja trouv�
            {
                if (CheckCollisionPointRec(GetMousePosition(), cards[i].hitbox))
                {
                    debugP("Clicked on card %d, id %d", i, cards[i].texture.id);
                    if (pair[0] == NULL)
                    {

                        if (!hardMode)
                            cards[i].revealed = true;

                        pair[0] = &cards[i];
                        PlaySound(soundShow);
                    }
                    else if (pair[1] == NULL) // Si une mais pas deux
                    {
                        click_time = GetTime();
                        essais++; // Incremente essai quand deux carte sont cliqu�es.

                        if (!hardMode)
                            cards[i].revealed = true;

                        pair[1] = &cards[i];
                        if (pair[0]->texture.id == pair[1]->texture.id) // Si elles sont identique
                        {
                            pair[0]->found = true;
                            pair[1]->found = true;

                            memset(pair, 0, 2 * sizeof(Card *));

                            found += 2;
                            if (found >= MAX_CARDS) // Victoire
                            {
                                state = Win;
                                PlaySound(soundVictory);

                                if (essais < gSaveData.bestScore || (essais == gSaveData.bestScore && temps < gSaveData.bestTime))
                                {
                                    SaveScore();
                                    sprintf(best_score, "\nEssais: %d\nTemps: %d\npar %s", gSaveData.bestScore, gSaveData.bestTime, gSaveData.userName);
                                }
                            }
                            else
                                PlaySound(soundFound);
                        }
                        else
                        {
                            PlaySound(soundShow);
                        }
                    }
                    break; // On sort de l'iteration
                }
                else
                    hideClickedCard();
            }
        }
    }
}

void pollKonami(void)
{
    for (int i = 0; i < sizeof(konamiCode) / sizeof(int); i++)
    {
        if (IsKeyPressed(konamiCode[i]))
        {
            if (konamiCode[i] == konamiCode[indexKonami++])
            {
                if (indexKonami == 10)
                {
                    debugP("Konami code!");
                    revealAllCard(true);
                    PlaySound(soundVictory);
                    state = Win;
                    indexKonami = 0;
                }
            }
            else
                indexKonami = 0;
            debugP("key: %d (%c) index: %d", konamiCode[i], (char)konamiCode[i], indexKonami);
            break;
        }
    }
}

void onResized(void)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    if (keepRatio)
    {
        // En fonction de l'axe le plus redimensionn�,
        // on redimensionne l'axe oppos�.
        if (abs(gameScreenWidth - width) > abs(gameScreenHeight - height))
            height = width / screenRatio;
        else
            width = screenRatio * height;

        SetWindowSize(width, height);
    }
    // On met � jour les nouvelles dimensions.
    gameScreenHeight = GetScreenHeight();
    gameScreenWidth = GetScreenWidth();

    heightRatio = (float)gameScreenWidth / (float)baseWidth;
    widthRatio = (float)gameScreenHeight / (float)baseHeight;

    debugP("Window resized to %dx%d", gameScreenWidth, gameScreenHeight);
    debugP("\b ratio: x: %f y: %f", widthRatio, heightRatio);

    for (int i = 0; i < MAX_CARDS; i++)
    {
        cards[i].hitbox.x = cards[i].pos.x;
        cards[i].hitbox.y = cards[i].pos.y;

        cards[i].hitbox.width = 10 * GetScreenWidth() / 100;
        ;
        cards[i].hitbox.height = 23 * GetScreenWidth() / 100;
        ;
    }
}

void board_loop(void)
{
    running = true;
    replay();

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            replay();

        if (IsKeyPressed(KEY_H))
            hideFoundCard = !hideFoundCard;

        if (IsKeyPressed(KEY_N))
            hardMode = !hardMode;

        pollKonami();

        if (state == Playing || state == Revealed)
        {
            if (GetTime() - clock_time > 1)
            {
                temps += GetTime() - clock_time;
                clock_time = GetTime();
            }
        }

        switch (state)
        {
        case Playing:
            updatePlayingState();
            break;
        case Revealed:

            if (GetTime() - timer > REVEALED_DURATION)
            {
                timer = GetTime();
                revealAllCard(false);
                state = Playing;
            }
            break;

        case Win:
            break;
        }
        drawBoard();
    }

    running = false;
}

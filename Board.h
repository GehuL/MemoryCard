#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

#define HRESOLUTION  768
#define WRESOLUTION  1024

#define WIDTH_GAP 50
#define HEIGHT_GAP 10

#define MAX_CARDS 16 // Doit etre multiple de 2

typedef enum
{
    PORTRAIT = 1,
    PAYSAGE = 2,
} Orientation;

typedef enum KEY_STATE
{
    PRESSED,
    RELEASED,
    DOWN,
} KEY_STATE;

typedef enum GameState
{
    Revealed,
    Playing,
    Win,
} GameState;

typedef struct Card
{
    Rectangle hitbox;
    Vector2 pos;
    Texture2D texture;
    bool revealed;
    bool found;
    int index;      // La frame dans le sprite
} Card;

typedef struct SaveData
{
    int bestScore;
    int bestTime;
    char userName[32];
} SaveData;

// Cr�er la fenetre, charge les ressources et initialize le plateau.
void board_load(int width, int height, const char* title);

// Gere les entr�es souris/clavier et evenements fenetre
// Execute la logique de la partie et les graphismes.
void board_loop(void);

// D�truit la fenetre et decharge les ressources.
void board_unload(void);

// Appelle le callback pour chaque touche press�, relach� ou enfonc�.
void board_keyboardCallback(void (*func)(int, KEY_STATE));

// Si Oui est true, la taille gardera le m�me ratio en fonction de l'orientation.
// Sinon orientation peut �tre ommis.
void board_keepRatio(bool oui, Orientation);

#endif // BOARD_H_INCLUDED

#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "include/Sprite.h"

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

// Créer la fenetre, charge les ressources et initialize le plateau.
void board_load(int width, int height, const char* title);

// Gere les entrées souris/clavier et evenements fenetre
// Execute la logique de la partie et les graphismes.
void board_loop(void);

// Détruit la fenetre et decharge les ressources.
void board_unload(void);

// Appelle le callback pour chaque touche pressé, relaché ou enfoncé.
void board_keyboardCallback(void (*func)(int, KEY_STATE));

// Si Oui est true, la taille gardera le même ratio en fonction de l'orientation.
// Sinon orientation peut être ommis.
void board_keepRatio(bool oui, Orientation);

#endif // BOARD_H_INCLUDED

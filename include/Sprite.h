#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED

#include "raylib.h"

typedef struct Sprite
{
    int frame; // L'image qui sera dessiner
    Rectangle source; // READ ONLY : Contient position, et longueur largeur dans la texture pour une image.
    Texture2D* texture; // Reference de la texture.
} Sprite;

Sprite LoadSprite(Texture2D* texture, int col, int row);

void SetFrame(Sprite* sprite, int frame);

#endif // SPRITE_H_INCLUDED

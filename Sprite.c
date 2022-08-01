#include "include/Sprite.h"

Sprite LoadSprite(Texture2D* texture, int col, int row)
{
    Sprite sprite = {0};

    sprite.source.width = texture->width / col;
    sprite.source.height = texture->height / row;

    sprite.texture = texture;

    return sprite;
}

void SetFrame(Sprite* sprite, int frame)
{
    int col = (sprite->texture->width / sprite->source.width);

    sprite->source.x = sprite->frame % col * sprite->source.width;
    sprite->source.y = sprite->frame / col * sprite->source.height;
}

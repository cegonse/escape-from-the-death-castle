#pragma once

#include <genesis.h>
#include "vector2.h"
#include "stage.h"

typedef enum character_animation {
    kCHARACTER_ANIMATION_IDLE = 0,
    kCHARACTER_ANIMATION_WALK,
    kCHARACTER_ANIMATION_AIR_SLASH,
    kCHARACTER_ANIMATION_JUMP_START,
    kCHARACTER_ANIMATION_JUMP_AIR,
    kCHARACTER_ANIMATION_JUMP_END,
    kCHARACTER_ANIMATION_JUMP_HIT,
} TCHARACTER_ANIMATION;

typedef enum character_direction {
    kCHARACTER_DIRECTION_NONE = 0,
    kCHARACTER_DIRECTION_LEFT,
    kCHARACTER_DIRECTION_RIGHT,
} TCHARACTER_DIRECTION;


typedef struct character {
    Sprite *sprite;
    u8 hflip;
    TCHARACTER_ANIMATION current_animation;
    u8 animation_frame;
    u8 wait_frame;

    struct vector2 position;
    struct vector2 velocity;
    struct vector2 size;

    struct vector2 bottom_collider;
    struct vector2 top_collider;
    struct vector2 left_collider;
    struct vector2 right_collider;

    u8 is_jumping;
    u8 is_attacking;
} Character;

void Character_Init(Character *self);

void Character_SetSprite(Character *self, const SpriteDefinition *sprite, u8 palette_index);

void Character_Update(Character *self, TCHARACTER_DIRECTION direction, const struct stage *stage);

void Character_OnJump(Character *self);

void Character_OnAttack(Character *self);

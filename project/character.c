#include "character.h"
#include <sys.h>

#define CHARACTER_IS_FALLING(p)        (p->velocity.y != 0)


static const fix16 direction_table[] = {
    [kCHARACTER_DIRECTION_NONE] = FIX16(0.0),
    [kCHARACTER_DIRECTION_LEFT] = FIX16(-0.8),
    [kCHARACTER_DIRECTION_RIGHT] = FIX16(0.8),
};

static const u8 direction_to_hflip[] = {
    [kCHARACTER_DIRECTION_LEFT] = 1,
    [kCHARACTER_DIRECTION_RIGHT] = 0,
};

static const u8 animation_length[] = {
    [kCHARACTER_ANIMATION_IDLE] = 3,
    [kCHARACTER_ANIMATION_WALK] = 6,
    [kCHARACTER_ANIMATION_AIR_SLASH] = 4,
};

static const u8 animation_speed[] = {
    [kCHARACTER_ANIMATION_IDLE] = 10,
    [kCHARACTER_ANIMATION_WALK] = 6,
    [kCHARACTER_ANIMATION_AIR_SLASH] = 4,
};

static fix16 kCHARACTER_JUMP_VELOCITY = FIX16(-4.0);
static fix16 kCHARACTER_FLOOR_LIMIT = FIX16(190.0);
static fix16 kCHARACTER_GRAVITY = FIX16(0.2);
static fix16 kCHARACTER_FRICTION = FIX16(0.8);
static fix16 kCHARACTER_MIN_VELOCITY = FIX16(0.3);
static fix16 kCHARACTER_MAX_GROUND_VELOCITY = FIX16(2.0);
static fix16 kCHARACTER_MAX_AIR_VELOCITY = FIX16(2.0);
static fix16 kCHARACTER_RIGHT_LIMIT = FIX16(290.0);
static fix16 kCHARACTER_LEFT_LIMIT = FIX16(10.0);


static void initializePalConstants(void)
{
    kCHARACTER_JUMP_VELOCITY = FIX16(-5.8);
    kCHARACTER_GRAVITY = FIX16(0.44);
    kCHARACTER_FRICTION = FIX16(0.82);
    kCHARACTER_MIN_VELOCITY = FIX16(0.36);
    kCHARACTER_MAX_GROUND_VELOCITY = FIX16(2.4);
    kCHARACTER_MAX_AIR_VELOCITY = FIX16(2.4);
}


void Character_Init(Character *self)
{
    memset(self, 0, sizeof(Character));
    self->position.x = FIX16(8);
    self->position.y = kCHARACTER_FLOOR_LIMIT - FIX16(10);
    self->size.x = FIX16(16);
    self->size.y = FIX16(32);
    self->is_attacking = 0;
    self->is_jumping = 0;

    self->bottom_collider.x = fix16Add(FIX16(8), fix16Div(self->size.x, FIX16(2)));
    self->bottom_collider.y = self->size.y;

    self->top_collider.x = fix16Add(FIX16(8), fix16Div(self->size.x, FIX16(2)));
    self->top_collider.y = FIX16(4);

    self->left_collider.x = FIX16(8);
    self->left_collider.y = fix16Div(self->size.y, FIX16(2));

    self->right_collider.x = fix16Sub(self->size.x, FIX16(8));
    self->right_collider.y = fix16Div(self->size.y, FIX16(2));

    if (IS_PALSYSTEM) {
        initializePalConstants();
    }
}


static inline void updateAnimation(Character *self, TCHARACTER_ANIMATION animation)
{
    if (self->current_animation != animation) {
        self->current_animation = animation;
        self->wait_frame = 0;
        self->animation_frame = 0;
        SPR_setAnim(self->sprite, self->current_animation);
    }
}


void Character_SetSprite(Character *self, const SpriteDefinition *sprite, u8 palette_index)
{
    if (self->sprite != NULL) {
        SPR_releaseSprite(self->sprite);
    }

    VDP_setPalette(PAL0 + palette_index, sprite->palette->data);

    self->sprite = SPR_addSprite(sprite, fix16ToInt(self->position.x), fix16ToInt(self->position.y), TILE_ATTR(PAL0 + palette_index, FALSE, FALSE, FALSE));

    updateAnimation(self, kCHARACTER_ANIMATION_IDLE);

    SPR_setVisibility(self->sprite, VISIBLE);
}


static inline void updatePosition(Character *self)
{
    self->position.x += self->velocity.x;
    self->position.y += self->velocity.y;

    SPR_setPosition(self->sprite, fix16ToInt(self->position.x), fix16ToInt(self->position.y));
}


static inline TSTAGE_COLLISION detectHorizontalCollision(Character *self, const struct stage *stage)
{
    struct vector2 collision_point;

    if (self->velocity.x < 0) {
        collision_point.x = self->position.x + self->left_collider.x;
        collision_point.y = self->position.y + self->left_collider.y;
    } else {
        collision_point.x = self->position.x + self->right_collider.x;
        collision_point.y = self->position.y + self->right_collider.y;
    }

    return Stage_CheckCollisions(&collision_point, self->velocity.x, 0, stage);
}


static inline void stopHorizontal(Character *self, const struct stage *stage)
{
    if (detectHorizontalCollision(self, stage) == kSTAGE_COLLISION) {
        self->velocity.x = 0;
        return;
    }

    if (abs(self->velocity.x) > kCHARACTER_MIN_VELOCITY) {
        if (self->velocity.x > 0) {
            self->velocity.x -= kCHARACTER_FRICTION;
        } else if (self->velocity.x < 0) {
            self->velocity.x += kCHARACTER_FRICTION;
        }
    } else {
        self->velocity.x = FIX16(0);
        updateAnimation(self, kCHARACTER_ANIMATION_IDLE);
    }
}


static inline void moveHorizontal(Character *self, TCHARACTER_DIRECTION direction, const struct stage *stage)
{   
    fix16 max_velocity = CHARACTER_IS_FALLING(self) ? kCHARACTER_MAX_AIR_VELOCITY : kCHARACTER_MAX_GROUND_VELOCITY;

    if (abs(self->velocity.x + direction_table[direction]) < max_velocity) {
        if (detectHorizontalCollision(self, stage) == kSTAGE_COLLISION) {
            self->velocity.x = 0;
            return;
        }

        self->velocity.x += direction_table[direction];
    }
}


static inline void clampToStageEdge(Character *self)
{
    if (self->position.x >= kCHARACTER_RIGHT_LIMIT) {
        self->position.x = kCHARACTER_RIGHT_LIMIT;
    }

    if (self->position.x <= kCHARACTER_LEFT_LIMIT) {
        self->position.x = kCHARACTER_LEFT_LIMIT;
    }
}


static inline void updateHorizontalVelocity(Character *self, TCHARACTER_DIRECTION direction, const struct stage *stage)
{
    if (direction == kCHARACTER_DIRECTION_NONE) {
        stopHorizontal(self, stage);
    } else {
        moveHorizontal(self, direction, stage);
    }

    clampToStageEdge(self);
}


static inline void clampToFloor(Character *self)
{
    if (self->velocity.y > 0) {
        self->velocity.y = 0;
        self->is_jumping = 0;
        self->position.y = kCHARACTER_FLOOR_LIMIT;
    }
}


static inline void updateFallingVelocity(Character *self, const struct stage *stage)
{
    struct vector2 collision_point;

    if (self->velocity.y < 0) {
        collision_point.x = self->position.x + self->top_collider.x;
        collision_point.y = self->position.y + self->top_collider.y;
    } else {
        collision_point.x = self->position.x + self->bottom_collider.x;
        collision_point.y = self->position.y + self->bottom_collider.y;
    }

    if (Stage_CheckCollisions(&collision_point, 0, self->velocity.y, stage) != kSTAGE_COLLISION) {
        self->velocity.y += kCHARACTER_GRAVITY;
    } else {
        self->velocity.y = 0;
        self->is_jumping = 0;
    }
}


static inline void updateVerticalVelocity(Character *self, const struct stage *stage)
{
    if (self->position.y < kCHARACTER_FLOOR_LIMIT) {
        updateFallingVelocity(self, stage);
    } else {
        clampToFloor(self);
    }
}


static inline void updateHorizontalFlip(Character *self, TCHARACTER_DIRECTION direction)
{
    u8 hflip;

    if (direction != kCHARACTER_DIRECTION_NONE && !self->is_attacking) {
        hflip = direction_to_hflip[direction];
        updateAnimation(self, kCHARACTER_ANIMATION_WALK);

        if (self->hflip != hflip) {
            self->hflip = hflip;
            SPR_setHFlip(self->sprite, hflip);
        }
    }
}


static inline void cycleAnimation(Character *self)
{
    self->wait_frame++;

    if (self->wait_frame == animation_speed[self->current_animation]) {
        self->wait_frame = 0;
        SPR_setFrame(self->sprite, self->animation_frame);
        self->animation_frame++;

        if (self->animation_frame == animation_length[self->current_animation]) {
            self->animation_frame = 0;

            if (self->is_attacking) {
                self->is_attacking = 0;
            }
        }
    }
}


void Character_Update(Character *self, TCHARACTER_DIRECTION direction, const struct stage *stage)
{
    updateHorizontalFlip(self, direction);

    updateHorizontalVelocity(self, direction, stage);

    updateVerticalVelocity(self, stage);

    cycleAnimation(self);

    updatePosition(self);
}


void Character_OnJump(Character *self)
{
    if (!self->is_jumping) {
        self->is_jumping = 1;
        self->velocity.y = kCHARACTER_JUMP_VELOCITY;
    }
}


void Character_OnAttack(Character *self)
{
    if (!self->is_attacking) {
        updateAnimation(self, kCHARACTER_ANIMATION_AIR_SLASH);
        self->is_attacking = 1;
    }
}
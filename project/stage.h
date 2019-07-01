/*
 * stage.h
 *
 *  Created on: 2 ene. 2018
 *      Author: jamoh
 */
#pragma once

#include <genesis.h>
#include "vector2.h"

#define kMAX_COLLIDERS		64


typedef enum stage_collision {
	kSTAGE_NO_COLLISION = 0,
	kSTAGE_COLLISION,
} TSTAGE_COLLISION;


struct collider {
	struct vector2i position;
	struct vector2i size;
};


struct stage {
	u16 num_colliders;
	struct collider colliders[kMAX_COLLIDERS];
	struct vector2i door;
};

u8 Stage_CheckCollisions(struct vector2 *position, fix16 dxf, fix16 dyf, const struct stage *stage);

void Stage_DebugDraw(const struct stage *stage);

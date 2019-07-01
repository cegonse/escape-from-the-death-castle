#include "stage.h"


u8 Stage_CheckCollisions(struct vector2 *position, fix16 dxf, fix16 dyf, const struct stage *stage)
{
	u8 i;
	u8 collision = kSTAGE_NO_COLLISION;
	const struct collider *collider;

	s16 x = fix16ToInt(position->x) + fix16ToInt(dxf);
	s16 y = fix16ToInt(position->y) + fix16ToInt(dyf);

	for (i=0; i<stage->num_colliders; ++i) {
		collider = &stage->colliders[i];

		collision =
		(
			(y < collider->position.y + collider->size.y) &&
			(y > collider->position.y)
		)
		&&
		(
			(x < collider->position.x + collider->size.x) &&
			(x > collider->position.x)
		);

		if (collision != kSTAGE_NO_COLLISION) {
			return collision;
		}
	}

	return collision;
}


void Stage_DebugDraw(const struct stage *stage)
{
	s16 x, y, h, w;
	u8 i, j, k;

	for (i=0; i<stage->num_colliders; ++i) {
		x = stage->colliders[i].position.x;
		y = stage->colliders[i].position.y;
		w = stage->colliders[i].size.x;
		h = stage->colliders[i].size.y;

		for (j=x; j != x+w; j+=8) {
			for (k=y; k != y+h; k+=8) {
				VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(PAL0 + 2,0,0,0,TILE_USERINDEX), j/8, k/8);
			}
		}
	}
}

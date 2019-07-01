#include <genesis.h>

#include "system.h"
#include "character.h"
#include "gfx.h"
#include "stage.h"

#define CONTROLLER_DIRECTION_MASK(ctl)  (ctl & 0xF)


static const struct stage test_stage = {
	.num_colliders = 3,
	.door = {
		.x = 0,
		.y = 0,
	},
	.colliders = {
		[0] = {
			.position.x = 48,
			.position.y = 200,
			.size.x = 64,
			.size.y = 24,
		},
		[1] = {
			.position.x = 144,
			.position.y = 160,
			.size.x = 64,
			.size.y = 16,
		},
		[2] = {
			.position.x = 216,
			.position.y = 120,
			.size.x = 32,
			.size.y = 16,
		},
	},
};


static Character main_character;
static u8 joy_state = 0;
static TCHARACTER_DIRECTION joy_to_direction[16] = {
	[0] = kCHARACTER_DIRECTION_NONE,
	[BUTTON_UP] = kCHARACTER_DIRECTION_NONE,
	[BUTTON_DOWN] = kCHARACTER_DIRECTION_NONE,
	[BUTTON_UP | BUTTON_LEFT] = kCHARACTER_DIRECTION_LEFT,
	[BUTTON_DOWN | BUTTON_LEFT] = kCHARACTER_DIRECTION_LEFT,
	[BUTTON_LEFT] = kCHARACTER_DIRECTION_LEFT,
	[BUTTON_UP | BUTTON_RIGHT] = kCHARACTER_DIRECTION_RIGHT,
	[BUTTON_DOWN | BUTTON_RIGHT] = kCHARACTER_DIRECTION_RIGHT,
	[BUTTON_RIGHT] = kCHARACTER_DIRECTION_RIGHT,
};


void joyCallback(u16 joy, u16 changed, u16 state)
{
	joy_state = state;
}


static inline void mainLoop(void)
{
	if (joy_state & BUTTON_A) {
		Character_OnJump(&main_character);
	}

	if (joy_state & BUTTON_B) {
		Character_OnAttack(&main_character);
	}

	Character_Update(&main_character, joy_to_direction[CONTROLLER_DIRECTION_MASK(joy_state)], &test_stage);
}


static inline void waitForVsync(void)
{
	char buffer[16];
	uintToStr(GET_VCOUNTER, buffer, 3);
	VDP_drawText(buffer, 0, 1);
	
	SPR_update();
	VDP_waitVSync();
}


int main()
{
	u8 detected_controllers;

	System_Init(&detected_controllers);

	JOY_setEventHandler(joyCallback);

	VDP_loadTileSet(&basic_tiles_def, TILE_USERINDEX, TRUE);
	VDP_setPalette(PAL2, basic_tiles_pal_def.data);

	Stage_DebugDraw(&test_stage);

	Character_Init(&main_character);
	Character_SetSprite(&main_character, &adventurer_def, 1);

	while (1) {
		mainLoop();
		waitForVsync();
	}

	return 0;
}

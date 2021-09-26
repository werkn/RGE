#ifndef __2D_RGE_SPRITE_EDITOR_SCENE
#define __2D_RGE_SPRITE_EDITOR_SCENE

#include "../RetroGE/RetroGE/rge.h"
#include "../sag_core.h"

#define SAG_SPRITE_ID_X 135
#define SAG_SPRITE_ID_Y 20

#define SAG_EDITOR_CANVAS_X 10
#define SAG_EDITOR_CANVAS_Y 20
#define SAG_EDITOR_CANVAS_W 120
#define SAG_EDITOR_CANVAS_H 120

#define SAG_EDITOR_PALETTE_X 10
#define SAG_EDITOR_PALETTE_Y 150
#define SAG_EDITOR_PALETTE_W 256
#define SAG_EDITOR_PALETTE_H 16

#define SAG_EDITOR_SPRITE_PAGE_X 10
#define SAG_EDITOR_SPRITE_PAGE_Y 175
#define SAG_EDITOR_SPRITE_PAGE_W 192
#define SAG_EDITOR_SPRITE_PAGE_H 48

typedef enum sag_sprite_editor_tool {
	PENCIL=0,
	FLOOD_FILL=1,
	END_OF_TOOLS=2  //indicates we need to reset on pencil
} sag_sprite_editor_tool_t;

scene_t scene_sprite_editor_init();
int sag_active_sprite_id();
void sag_flip_sprite(int sprite_id, bool h_flip, bool v_flip);
void sag_rotate_sprite(int sprite_id);
void sag_move_sprite(int sprite_id, sag_movement_t direction);
void sag_flood_fill(int replace_color_id, int fill_color_id, int x, int y, int sprite_id);
void sag_select_next_tool();

//input handling methods
void sag_canvas_input();
void sag_move_canvas_input();
void sag_bit_flags_input();
void sag_use_active_tool_input();
void sag_anim_input();


#endif

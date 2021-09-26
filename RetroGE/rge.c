#include "rge.h"

///////////////////////////////////////////////////////////////////////////////
// RGE GLOBALS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_GLOBALS_BLOCK
BITMAP* ui_layer;
BITMAP* sprite_layer;
BITMAP* background_layer;
BITMAP* back_buffer;
BITMAP* vram;  //TODO: wipe vram after loading all sprites
BITMAP* sprites[256];  //for now we just mirror out vram to sprites for access to allegro sprite functions
u8 flags_for_sprites[256];  //bits 0 - 7 set for each sprites
#ifdef RGE_DEBUG
	BITMAP* debug_layer;
#endif

#ifdef RGE_DEBUG
	volatile debug_t rge_debug;
#endif
volatile bool rge_shutdown = false;
volatile bool frame_tick = true;
//volatile bool audio_tick = true;
volatile bool input_tick = true;
volatile int global_timer_tick = 0;  //TODO: this needs to be set to 32/64 bit so we can monitor for overflow when timer exhausted

graphics_t rge_gfx;
input_state_t rge_input_state;
scene_manager_t rge_scene_manager;
ui_manager_t rge_ui_manager;
audio_manager_t rge_audio_manager;
#endif  //END __RGE_ENGINE_GLOBALS_BLOCK

///////////////////////////////////////////////////////////////////////////////
// DEBUG SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_DEBUG_SYSTEM_BLOCK
#ifdef RGE_DEBUG
	void rge_debug_reset() {
		rge_debug.frame_rate = rge_debug.frame_ticks_sec;
		rge_debug.frame_ticks_sec = 0;
		rge_debug.main_ops = rge_debug.main_ticks_sec;
		rge_debug.main_ticks_sec = 0;
	}
	END_OF_FUNCTION(rge_debug)

	void rge_debug_text(u16 x, u16 y, const char* message, text_align_t text_alignment) {
		switch (text_alignment) {
		case TEXT_ALIGN_LEFT:
			textprintf_ex(debug_layer, font, x, y, RGE_DEBUG_TEXT_FOREGROUND, RGE_DEBUG_TEXT_BACKGROUND, message);
			break;
		case TEXT_ALIGN_CENTER:
			textprintf_centre_ex(debug_layer, font, x, y, RGE_DEBUG_TEXT_FOREGROUND, RGE_DEBUG_TEXT_BACKGROUND, message);
			break;
		case TEXT_ALIGN_RIGHT:
			textprintf_right_ex(debug_layer, font, x, y, RGE_DEBUG_TEXT_FOREGROUND, RGE_DEBUG_TEXT_BACKGROUND, message);
			break;
		}
	
	}

	void rge_debug_palette() {

		rge_clear();

		u16 palette_index = 0;
		char index_text[4]; //ie: 256\0

		//rge_debug.message is limited to DEBUG_MSG_SIZE char, don't overflow :)
		for (u8 x = 0; x < 20; x++) {
			for (u8 y = 0; y < 13; y++) {
				rectfill(debug_layer, x * 16, y * 16, (x * 16) + 16, (y * 16) + 16, palette_index);
				//convert to hex so it will fit in tile
				// "0x%x\n", a
				snprintf(index_text, 3, "%x", palette_index);
				rge_debug_text(x * 16, y * 16, index_text, TEXT_ALIGN_LEFT);

				//only render 256 colors in palette
				if (++palette_index > 255) { 
					break; 
				}
			}
		}
	}

	//draw vram to display, assumes 640x480 resolution
	void rge_debug_vram() {
		//check vram is set
		if (vram) {
			rge_clear();
			//sprite files are always 128x128 @ 8x8 pixels making up 256 total tiles
			masked_stretch_blit(vram, debug_layer, 0, 0, vram->w, vram->h,
				0, 0, 128, 128);

			//draw info about whats under the mouse cursor
			//scale screen mouse pos to RGE_BASE_SCREEN_WIDTH / RGE_BASE_SCREEN_HEIGHT
			u16 mouse_x_scaled = ((float)mouse_x / SCREEN_W) * RGE_BASE_SCREEN_WIDTH;
			u16 mouse_y_scaled = ((float)mouse_y / SCREEN_H) * RGE_BASE_SCREEN_HEIGHT;
			
			//clamp mouse size to 128x128 vram
			if (mouse_x_scaled > 127) { mouse_x_scaled = 127; }
			if (mouse_y_scaled > 127) { mouse_y_scaled = 127; }

			//sprite id 
			u8 sprite_column = ((mouse_x_scaled) / 8);
			u8 sprite_row = ((mouse_y_scaled) / 8);
			u8 sprite_id = sprite_column + (sprite_row * 16);

			char sprite_id_str[16];
			snprintf(sprite_id_str, 16, "SPRITE #: %d", sprite_id);
			rge_debug_text(132, 190, sprite_id_str, TEXT_ALIGN_LEFT);

			//indicate on vram bitmap where the sprite in question is
			rge_gfx.stroke_color = 255;
			rge_draw_rect_no_fill(debug_layer, sprite_column * 8, sprite_row * 8, 8, 8);

			//lastly render sprite scaled up with ID
			masked_stretch_blit(vram, debug_layer, sprite_column * 8, sprite_row * 8, 8, 8, 132, 0, 188, 188);
		}
		else {
			rge_debug_text(RGE_BASE_SCREEN_WIDTH/2, RGE_BASE_SCREEN_HEIGHT/2, "<NOTHING IN VRAM>", TEXT_ALIGN_CENTER);
		}
	}

	//add a message to the debug overlay message queue
	void rge_set_debug_msg(u8 debug_msg_index, const char* msg) {
		//check length is OK
		if (debug_msg_index >= 0 && debug_msg_index < DEBUG_MSG_COUNT - 1) {
			//safe copy in msg
			snprintf(rge_debug.message[debug_msg_index], DEBUG_MSG_SIZE, "%s", msg);
		}
	}
#endif
#endif  //END __RGE_ENGINE_DEBUG_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// SAVE AND SETTINGS SYSTEMS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_SAVE_AND_SETTINGS_BLOCK
//-----------------------------------------------------------------------------
// SETTING (Single Setting)
//-----------------------------------------------------------------------------
void rge_setting_init(rge_setting_t* setting, rge_settings_type_t type, char* key, char* value) {
	setting->type = type;
	setting->key = new_string(key);
	setting->value = new_string(value);
}
void rge_setting_update(rge_setting_t* setting, char* new_value) {
	update_string(setting->value, new_value);
}

void rge_setting_dispose(rge_setting_t* setting) {
	dispose_string(setting->key);
	dispose_string(setting->value);
}

//-----------------------------------------------------------------------------
// SETTINGS (Collection w/management)
//-----------------------------------------------------------------------------
void rge_settings_init(rge_settings_t* settings) {
	settings->add_setting = rge_settings_add_setting;
	settings->update_setting = rge_settings_update_setting;
	settings->dispose_setting = rge_settings_dipose_setting;
	settings->get_int_setting = rge_settings_get_int_setting;
	settings->get_bool_setting = rge_settings_get_bool_setting;
	settings->get_string_setting = rge_settings_get_string_setting;

	//init arraystack
	arraystack_init(&settings->settings, sizeof(rge_settings_t));
}

void rge_settings_add_setting(struct rge_settings* settings, rge_settings_type_t type, char* key, char* value) {
#ifdef RGE_DEBUG
	//check key is not in use
	if (rge_settings_is_key_in_use(settings, key)) {
		assert("Settings key provided is already in use, you've likely tried to add a setting with the same key." && false);
	}
#endif 
	rge_setting_t tmp;
	rge_setting_init(&tmp, type, key, value);
	arraystack_push(&settings->settings, &tmp);
}

void rge_settings_update_setting(struct rge_settings* settings, char* key, char* new_value) {
#ifdef RGE_DEBUG
	//check key exists
	if (!rge_settings_is_key_in_use(settings, key)) {
		assert("Settings key provided for update does not exist." && false);
	}
#endif

	if (settings->settings.length > 0) {
		arraystack_iterator(&settings->settings, &settings->iterator, 0, settings->settings.length - 1);
		while (settings->iterator.next(&settings->iterator)) {
			rge_setting_t* current_setting = (rge_setting_t*)settings->iterator.elem(&settings->iterator);
			if (strcmp(current_setting->key->text, key) == 0) {
				update_string(current_setting->value, new_value);
			}
		}

		//clean up stack/iterator
		settings->iterator.dispose(&settings->iterator);
	}
}

void rge_settings_dipose_setting(struct rge_settings* settings, char* key) {
	int index = 0;
	
	//only create an iterator if we have components in the manager
	if (settings->settings.length > 0) {
		arraystack_iterator(&settings->settings, &settings->iterator, 0, settings->settings.length - 1);
		//delete each setting 
		while (settings->iterator.next(&settings->iterator)) {
			rge_setting_t* current_setting = (rge_setting_t*)settings->iterator.elem(&settings->iterator);
			if (strcmp(current_setting->key->text, key) == 0) {
				rge_setting_dispose(current_setting);

				//now remove from arraystack
				arraystack_remove(&settings->settings, index, NULL);
			}
			index++;
		}

		//clean up stack/iterator
		settings->iterator.dispose(&settings->iterator);
	}
}

bool rge_settings_is_key_in_use(struct rge_settings* settings, char* key) {
	bool key_found = false;
	
	if (settings->settings.length > 0) {
		arraystack_iterator(&settings->settings, &settings->iterator, 0, settings->settings.length - 1);
		while (settings->iterator.next(&settings->iterator)) {
			rge_setting_t* current_setting = (rge_setting_t*)settings->iterator.elem(&settings->iterator);
			if (strcmp(current_setting->key->text, key) == 0) {
				key_found = true;
				break;
			}
		}

		//clean up stack/iterator
		settings->iterator.dispose(&settings->iterator);
	}

	return key_found;
}

int  rge_settings_get_int_setting(struct rge_settings* settings, char* key) {

	int ret;
	bool found_key = false;

	if (settings->settings.length > 0) {
		arraystack_iterator(&settings->settings, &settings->iterator, 0, settings->settings.length - 1);
		while (settings->iterator.next(&settings->iterator)) {
			rge_setting_t* current_setting = (rge_setting_t*)settings->iterator.elem(&settings->iterator);
			if (strcmp(current_setting->key->text, key) == 0) {
				ret = atoi(current_setting->value->text);
				found_key = true;
				break;
			}
		}

		//clean up stack/iterator
		settings->iterator.dispose(&settings->iterator);
	}
#ifdef RGE_DEBUG
	if (!found_key) {
		assert("Request for non-existent setting key in scene." && false);
	}
#endif

	return ret;
}

bool rge_settings_get_bool_setting(struct rge_settings* settings, char* key) {
	bool ret = false;
	bool found_key = false;

	if (settings->settings.length > 0) {
		arraystack_iterator(&settings->settings, &settings->iterator, 0, settings->settings.length - 1);
		while (settings->iterator.next(&settings->iterator)) {
			rge_setting_t* current_setting = (rge_setting_t*)settings->iterator.elem(&settings->iterator);
			if (strcmp(current_setting->key->text, key) == 0) {
				
				if (strcmp(current_setting->value->text, "true") == 0) {
					ret = true;
				}

				found_key = true;
				break;
			}
		}

		//clean up stack/iterator
		settings->iterator.dispose(&settings->iterator);
	}
#ifdef RGE_DEBUG
	if (!found_key) {
		assert("Request for non-existent setting key in scene." && false);
	}
#endif

	return ret;
}

char* rge_settings_get_string_setting(struct rge_settings* settings, char* key) {
	bool found_key = false;
	rge_string_t* value;

	if (settings->settings.length > 0) {
		arraystack_iterator(&settings->settings, &settings->iterator, 0, settings->settings.length - 1);
		while (settings->iterator.next(&settings->iterator)) {
			rge_setting_t* current_setting = (rge_setting_t*)settings->iterator.elem(&settings->iterator);
			if (strcmp(current_setting->key->text, key) == 0) {
				found_key = true;
				//clean up stack/iterator
				settings->iterator.dispose(&settings->iterator);
				return current_setting->value->text;
			}
		}

	}
#ifdef RGE_DEBUG
	if (!found_key) {
		assert("Request for non-existent setting key in scene." && false);
	}
#endif
	return NULL;
}

void rge_settings_dispose(rge_settings_t* settings) {
	//only create an iterator if we have components in the manager
	if (settings->settings.length > 0) {
		arraystack_iterator(&settings->settings, &settings->iterator, 0, settings->settings.length - 1);
		//delete each setting 
		while (settings->iterator.next(&settings->iterator)) {
			rge_setting_t* setting_to_dispose = (rge_setting_t*)settings->iterator.elem(&settings->iterator);
			rge_setting_dispose(setting_to_dispose);
		}

		//clean up stack/iterator
		settings->iterator.dispose(&settings->iterator);
	}
	//delete arraystack
	arraystack_dispose(&settings->settings);
}
#endif  //END __RGE_ENGINE_SAVE_AND_SETTINGS_BLOCK

///////////////////////////////////////////////////////////////////////////////
// GRAPHICS SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_GRAPHICS_SYSTEM_BLOCK

void rge_draw_text(BITMAP* layer, int x, int y, const char* message, color_palette_index_t foreground, color_palette_index_t background, text_align_t text_alignment) {
	switch (text_alignment) {
	case TEXT_ALIGN_LEFT:
		textprintf_ex(layer, font, x, y, foreground, background, message);
		break;
	case TEXT_ALIGN_CENTER:
		textprintf_centre_ex(layer, font, x, y, foreground, background, message);
		break;
	case TEXT_ALIGN_RIGHT:
		textprintf_right_ex(layer, font, x, y, foreground, background, message);
		break;
	}
}

void rge_draw_sprite(BITMAP* layer, int x, int y, int sprite_id, bool h_flip, bool v_flip) {
	switch (h_flip) {
		case false:
			switch (v_flip) {
				//don't flip horizontal or vertical, most likely case hence first in switch
				case false:
					draw_sprite_ex(layer, sprites[sprite_id], x, y, DRAW_SPRITE_NORMAL, DRAW_SPRITE_NO_FLIP);
					break;
				//flip vertical but not horizontal
				case true:
					draw_sprite_ex(layer, sprites[sprite_id], x, y, DRAW_SPRITE_NORMAL, DRAW_SPRITE_V_FLIP);
					break;
			}
			break;

		case true:
			switch (v_flip) {
				//flip horizontal but not vertical
				case false:
					draw_sprite_ex(layer, sprites[sprite_id], x, y, DRAW_SPRITE_NORMAL, DRAW_SPRITE_H_FLIP);
					break;
				//flip horizontal and vertical
				case true:
					draw_sprite_ex(layer, sprites[sprite_id], x, y, DRAW_SPRITE_NORMAL, DRAW_SPRITE_VH_FLIP);
					break;
			}
	}
	
}

//currently does not support flipping when scaled
void rge_draw_scaled_sprite(BITMAP* layer, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int sprite_id) {
	masked_stretch_blit(sprites[sprite_id], sprite_layer, sx, sy, sw, sh, dx, dy, dw, dh);
}

//rotates a sprite 90 clockwise or counter clockwise
void rge_draw_rotated_sprite(BITMAP* src, BITMAP* dest, int x, int y, bool clockwise) {
	switch (clockwise) {
	case true:
		rotate_sprite(dest, src, 0, 0, itofix(64));
		break;
	case false:
		rotate_sprite(dest, src, 0, 0, itofix(-64));
		break;
	}
}

void rge_draw_rect(BITMAP* layer, int x, int y, int width, int height) {
	rectfill(layer, x, y, x + width, y + height , rge_gfx.fill_color);
}

void rge_draw_rect_no_fill(BITMAP* layer, int x, int y, int width, int height) {
	rect(layer, x, y, x + width, y + height, rge_gfx.stroke_color);
}

void rge_draw_circle(BITMAP * layer, int x, int y, int radius) {
	ellipsefill(layer, x + radius, y + radius, radius, radius, rge_gfx.fill_color);
}

void rge_draw_circle_no_fill(BITMAP* layer, int x, int y, int radius) {
	ellipse(layer, x + radius, y + radius, radius, radius, rge_gfx.stroke_color);
}

void rge_draw_line(BITMAP* layer, int x1, int y1, int x2, int y2) {
	line(layer, x1, y1, x2, y2, rge_gfx.stroke_color);
}

void rge_draw_triangle(BITMAP* layer, int x1, int y1, int x2, int y2, int x3, int y3) {
	line(layer, x1, y1, x2, y2, rge_gfx.stroke_color);
	line(layer, x2, y2, x3, y3, rge_gfx.stroke_color);
	line(layer, x3, y3, x1, y1, rge_gfx.stroke_color);
}

void rge_draw_triangle_no_fill(BITMAP* layer, int x1, int y1, int x2, int y2, int x3, int y3) {
	triangle(layer, x1, y1, x2, y2, x3, y3, rge_gfx.fill_color);
}

void rge_draw_1bpp_sprite(BITMAP* layer, sprite_1bpp_t sprite, int x, int y) {
	//each row of bytes represents y pos, so byte = 0 first row, byte = 7 last row
	for (int byte = 0; byte < FORMAT_1BPP; byte++) {
		for (int px_x = 0; px_x < 8; px_x++) {
			switch (get_byte_flag(sprite.sprite[7 - byte], px_x)) {
			//pixel is set, draw with stroke color
			case true:
				rge_set_pixel(layer, x + px_x, y+byte, rge_gfx.stroke_color);
				break;
			//pixel is not set, draw with clear color
			case false:
				rge_set_pixel(layer, x + px_x, y + byte, rge_gfx.clear_color);
				break;
			}
		}
	}
}

void rge_draw_2bpp_sprite(BITMAP* layer, sprite_2bpp_t sprite, int x, int y) {

}

void rge_draw_4bpp_sprite(BITMAP* layer, sprite_4bpp_t sprite, int x, int y) {

}

void rge_set_pixel(BITMAP* layer, int x, int y, int color_id) {
	_putpixel(layer, x, y, color_id);
}

int rge_get_pixel(BITMAP* layer, int x, int y) {
	return _getpixel(layer, x, y);
}

void rge_flip_backbuffer() {

	rge_scene_manager.draw();
	rge_ui_manager.draw();

	//BLIT MASK our 3 layers (background, sprite, and ui) to backbuffer and then
	//backbuffer to screen
	masked_blit(background_layer, back_buffer, 0, 0, 0, 0, RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_WIDTH);
	masked_blit(sprite_layer, back_buffer, 0, 0, 0, 0, RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_WIDTH); 
	masked_blit(ui_layer, back_buffer, 0, 0, 0, 0, RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_WIDTH); 

#ifdef RGE_DEBUG
	//tick frame (we always draw a frame when flipping buffer)
	//used to report FPS in debug overlay
	rge_debug.frame_ticks_sec++;

	//DEBUG will overlay all other layers if active
	if (rge_debug.visible) {
		
		//we display palette or VRAM debug but not both at the same time
		switch (rge_debug.debug_mode) {
		case DEBUG_PALETTE:
			rge_debug_palette(RGE_PALETTES);
			break;
		case DEBUG_VRAM:
			rge_debug_vram();
			break;
		}

		//rge_debug.message is limited to DEBUG_MSG_SIZE char, don't overflow :)
		snprintf(rge_debug.message[DEBUG_MSG_COUNT - 1], DEBUG_MSG_SIZE, "MSG(%d) [ FPS: %d MAIN OPS: %d ]", rge_debug.active_message_index, rge_debug.frame_rate, rge_debug.main_ops);

		rge_debug_text(0, DEBUG_MODE_TEXT_AREA_START_Y, rge_debug.message[DEBUG_MSG_COUNT - 1], TEXT_ALIGN_LEFT);

		//Print message queue (other than FPS)
		rge_debug_text(0, DEBUG_MODE_TEXT_AREA_START_Y + 10, rge_debug.message[rge_debug.active_message_index], TEXT_ALIGN_LEFT);

		//scale screen mouse pos to RGE_BASE_SCREEN_WIDTH / RGE_BASE_SCREEN_HEIGHT
		u16 mouse_x_scaled = ((float)mouse_x / SCREEN_W) * RGE_BASE_SCREEN_WIDTH;
		u16 mouse_y_scaled = ((float)mouse_y / SCREEN_H) * RGE_BASE_SCREEN_HEIGHT;
		u16 draw_under_cursor_info_x = 0;

		char mouse_pos_as_str[18];
		u8 color_index;
		
		switch (rge_debug.debug_mode) {
		case DEBUG_NO_OVERLAY:
			//give info on whats currently rendered to screen
			color_index = _getpixel(back_buffer, mouse_x_scaled, mouse_y_scaled);
			break;
		default:
			color_index = _getpixel(debug_layer, mouse_x_scaled, mouse_y_scaled);
			break;
		}
		snprintf(mouse_pos_as_str, 18, "  (%d,%d)->%d", mouse_x_scaled, mouse_y_scaled, color_index);
		text_align_t text_align_mode = TEXT_ALIGN_LEFT;

		if (mouse_x_scaled > 160) {
			draw_under_cursor_info_x = mouse_x_scaled - 2 - DEBUG_TILE_SIZE;
			text_align_mode = TEXT_ALIGN_RIGHT;
		}
		else {
			draw_under_cursor_info_x = mouse_x_scaled + 3;
			text_align_mode = TEXT_ALIGN_LEFT;
		}

		rge_debug_text(draw_under_cursor_info_x, mouse_y_scaled, mouse_pos_as_str, text_align_mode);
		//draw mouse pointer at px being read
		rect(debug_layer, mouse_x_scaled, mouse_y_scaled, mouse_x_scaled + 1, mouse_y_scaled + 1, RGE_DEBUG_TEXT_BACKGROUND);
		//draw color under cursor just to be more clear
		rectfill(debug_layer, draw_under_cursor_info_x, mouse_y_scaled, 
			draw_under_cursor_info_x + DEBUG_TILE_SIZE, 
			mouse_y_scaled + DEBUG_TILE_SIZE, color_index);
		rect(debug_layer, draw_under_cursor_info_x, mouse_y_scaled,
			draw_under_cursor_info_x + DEBUG_TILE_SIZE,
			mouse_y_scaled + DEBUG_TILE_SIZE, RGE_DEBUG_TEXT_BACKGROUND);

		masked_blit(debug_layer, back_buffer, 0, 0, 0, 0, RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_HEIGHT);
	}
#endif

	//FLIP BACKBUFFER AND SCALE UP TO SCREEN RES FROM 320x240
	masked_stretch_blit(back_buffer, screen, 0, 0, RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_HEIGHT,
		0, 0, SCREEN_W, SCREEN_H);

	//end redraw
	frame_tick = false;
}

void rge_clear() {
	//TODO add code to only clear if changes have occured in each of the layers
	clear_to_color(background_layer, bitmap_mask_color(background_layer));
	clear_to_color(sprite_layer, bitmap_mask_color(sprite_layer));
	clear_to_color(ui_layer, bitmap_mask_color(ui_layer));

#ifdef RGE_DEBUG
	clear_to_color(debug_layer, bitmap_mask_color(debug_layer));
#endif 

	//allways clear backbuffer
	clear_to_color(back_buffer, rge_gfx.clear_color);
}

#endif  //END __RGE_ENGINE_GRAPHICS_SYSTEM_BLOCK 

///////////////////////////////////////////////////////////////////////////////
// ENGINE LIFECYCLE
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_LIFECYCLE_BLOCK

//NOTE about ASYNC timers with ALLEGRO 4.4
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//IF we are using a function as an async timer callback
//we need to lock var/func for concurrent access.
// 
//To do this with Allegro 
//1.  Mark varaibles modified in function callbacks
//volatile like (frame_tick, audio_tick, and input_tick) below.
// 
//2.  Following function definition we add 
//END_OF_FUNCTION(func_name)
//
//3.  Later when initializing/installing a timer 
//using install_int(...)
//we preceed with LOCK_VARIABLE / LOCK_FUNCTION
//
//Thoughts:  
//I don't really get these but they seem similar to 
//annotations for Java and inform the build process 
//not to optimise and that these variables methods are 
//over multiple threads/may be modified outside main
//
//Reference: `/examples/extimer.c` from official examples

void rge_redraw() { frame_tick = true; }
END_OF_FUNCTION(rge_redraw)

//Why is rge_buffer_audio commented out?
//because under Allegro we don't manually buffer 
//any audio, when we switch to our sequencer
//we will need to change this
//void rge_buffer_audio() { audio_tick = true; }
//END_OF_FUNCTION(rge_buffer_audio)

void rge_poll_input() { input_tick = true; }
END_OF_FUNCTION(rge_poll_input)

void rge_global_timer_tick() { global_timer_tick++; }
END_OF_FUNCTION(rge_global_timer_tick)

rge_state_t rge_init(screen_resolution_t resolution, const char* sag_path)
{
	//we only support 4:3 aspect ratio resolutions (Allegro 4.4 seems to crash if you use something else)
	//we also only support up to 1280/760, again Allegro 4.4 seems to crash past that

	u16 width;
	u16 height;
	switch (resolution) {
	case RESOLUTION_320x240:
		width = 320;
		height = 240;
		break;
	default:
	case RESOLUTION_640x480:
		width = 640;
		height = 480;
		break;
	/* Broke in Windows 10 as of 6/16/2021
	case RESOLUTION_1280x960:
		width = 1280;
		height = 960;
		break;
	*/
	}

	//Don't do anything before Allegro init
	if (allegro_init() != 0) return RGE_STATE_AL4_INIT_FAILED;
	set_color_depth(RGE_COLOR_DEPTH);

	//4.4.2 under Windows 10 (these are fixed in 4.4.3 I think) will crash and has a bunch of fullscreen issues
	if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, width, height, 0, 0) != 0)
		return RGE_STATE_GFX_INIT_FAILED;

	//set screen size
	rge_gfx.screen_width = width;
	rge_gfx.screen_height = height;

	//set game size (always should be 320x240)
	rge_gfx.game_width = RGE_BASE_SCREEN_WIDTH;
	rge_gfx.game_height = RGE_BASE_SCREEN_HEIGHT;

	rge_gfx.clear_color = RGE_CLEAR_COLOR_INDEX;
	rge_gfx.stroke_color = RGE_DEFAULT_FOREGROUND_INDEX;

	//assign function pointers
	rge_gfx.clear = rge_clear;
	rge_gfx.flip_backbuffer = rge_flip_backbuffer;
	rge_gfx.ellipse = rge_draw_circle_no_fill;
	rge_gfx.triangle = rge_draw_triangle_no_fill;
	rge_gfx.rect = rge_draw_rect;
	rge_gfx.rect_nofill = rge_draw_rect_no_fill;
	rge_gfx.text = rge_draw_text;

	//attempt to load the RGE palette (which contains multiple palettes)
	//representing older systems like the GameBoy, CGA, etc...
	set_pallete(RGE_PALETTES);

	//create our 3 layers (background, sprite and ui)
	background_layer = create_bitmap(RGE_BACKGROUND_MAP_TILE_X * RGE_TILE_SIZE_W,
		RGE_BACKGROUND_MAP_TILE_Y * RGE_TILE_SIZE_H);
	sprite_layer = create_bitmap(RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_HEIGHT);
	ui_layer = create_bitmap(RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_HEIGHT);

	//create backbuffer
	back_buffer = create_bitmap(SCREEN_W, SCREEN_H);

	//load spritemap into vram and pack into sprites[] array
	//populate VRAM, must be 128x128 and assumed to be 8x8 pixel tiles
	//load our sag file from disk
	vram = create_bitmap(128, 128);
	//clear it (because it likely has some random data in it)
	clear_to_color(vram, rge_gfx.clear_color);
	
	switch (import_sag_file(FORMAT_1BPP, sag_path, SAG)) {
	case FILE_ERROR_UNKNOWN_NOT_IMPORTED:
		assert("Unable to import provided SAG file" && false);
		break;
	}

#ifdef RGE_DEBUG
	debug_layer = create_bitmap(RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_HEIGHT);
	//set debug queue messages
	rge_set_debug_msg(0, "0 UNUSED ");
	rge_set_debug_msg(1, "1 UNUSED ");
	rge_set_debug_msg(2, "2 UNUSED ");
	rge_set_debug_msg(3, "3 UNUSED ");
#endif

	//init audio
	rge_audio_init();
	rge_input_init();

	//attempt to setup timer system
	if (install_timer() != 0) return RGE_STATE_TIMER_INIT_FAILED;

	//START SETUP OF ASYNC CALLBACKS and TIMERS
	//first begin by locking variables and functions that are used in interrupts
	LOCK_VARIABLE(frame_tick);
	//LOCK_VARIABLE(audio_tick);  //we lock audio tick here as were just using the Allegro audio system
	LOCK_VARIABLE(input_tick);
	LOCK_VARIABLE(global_timer_tick);
	LOCK_FUNCTION(rge_redraw);
	LOCK_FUNCTION(rge_buffer_audio);
	LOCK_FUNCTION(rge_poll_input);
	LOCK_FUNCTION(rge_global_timer_tick);

	//setup callbacks for handling ticks of input, audio and graphics
	if (install_int(rge_redraw, REDRAW_MS) != 0) return RGE_STATE_FRAME_TIMER_INIT_FAILED;
	if (install_int(rge_poll_input, INPUT_MS) != 0) return RGE_STATE_INPUT_TIMER_INIT_FAILED;
	if (install_int(rge_global_timer_tick, GLOBAL_TIMER_TICKS_MS) != 0) return RGE_STATE_INPUT_TIMER_INIT_FAILED;

	//setup debug timer for frame timing / main ops
#ifdef RGE_DEBUG
	LOCK_VARIABLE(rge_debug);
	LOCK_FUNCTION(rge_debug_reset);
	if (install_int(rge_debug_reset, ONE_SECOND_MS) != 0) return RGE_STATE_DEBUG_TIMER_INIT_FAILED;
#endif 

	//END SETUP OF ASYNC CALLBAKCS AND TIMERS

	//init various managers
	rge_scene_manager_init();
	rge_ui_manager_init();
	rge_audio_manager_init();

	//seed random number generator
	srand(time(NULL));

	return RGE_STATE_OK;
}

void rge_update() {

	rge_scene_manager.update();
	rge_ui_manager.update();

#ifdef RGE_DEBUG
	//tick frame (we always draw a frame when flipping buffer)
	//used to report FPS in debug overlay
	rge_debug.main_ticks_sec++;
#endif
}

void rge_dispose() {
	//destroy back_buffer and other layers
	destroy_bitmap(ui_layer);
	destroy_bitmap(sprite_layer);
	destroy_bitmap(background_layer);
#ifdef RGE_DEBUG
	destroy_bitmap(debug_layer);
#endif
	destroy_bitmap(vram);
	//destroy sprites
	for (int i = 0; i < 256; i++) {
		destroy_bitmap(sprites[i]);
	}

	destroy_bitmap(back_buffer);

	//dispose of ui system and scene system
	rge_scene_manager.dispose();
	rge_ui_manager.dispose();
	rge_audio_manager.dispose();

	//unload sound support
	remove_sound();

	//we don't need to explicitly call
	//as docs mention allegro_init() has its own callback
	//but doing because I've seen most examples have a call
	allegro_exit();

	//check for memory leaks
#if RGE_DEBUG
	_CrtDumpMemoryLeaks();
#endif
}


#endif  //END __RGE_ENGINE_LIFECYCLE_BLOCK 

///////////////////////////////////////////////////////////////////////////////
// INPUT SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_INPUT_SYSTEM_BLOCK

rge_state_t rge_input_init() {

	//attempt to setup input
	if (install_keyboard() != 0) return RGE_STATE_INPUT_INIT_FAILED;

#ifdef RGE_DEBUG
	if (install_mouse() != 0) return RGE_STATE_INPUT_INIT_FAILED;
	//lock to our 320x240 system
	set_mouse_range(0, 0, RGE_BASE_SCREEN_WIDTH - 1, RGE_BASE_SCREEN_HEIGHT - 1);
#endif
	return RGE_STATE_OK;
}


//returns whether button was released since last check and resets release flag
bool rge_button_was_released(button_input_state_t* button) {
	bool released = button->released;
	button->released = false;
	return released;
}

void rge_enable_button_pressed_duration(button_input_state_t* button, u8 max_seconds, u8 tick_decay_rate, u8 tick_growth_rate) {
	button->tick_decay_rate = tick_decay_rate;
	button->tick_growth_rate = tick_growth_rate;
	button->max_tick_count = max_seconds * (ONE_SECOND_MS / INPUT_MS);
}

void rge_set_button_state(button_input_state_t* button, char bound_key_state) {
	switch (bound_key_state) {
	case -1: //allegro uses -1 for true...
		if (button->pressed) {
			//button already pressed, tick pressed_ticks, 0 - 255 @ 50 ms input timing means we get 20 
			//input polls per second or 255/20 = 12 seconds of pressed duration handling
			//init all buttons have max_tick_count = 0 so we don't update pressed_ticks
			//to enable pressed ticks we set max_tick_count directly on the button we want to watch
			switch (rge_input_state.mode) {
			case SIMPLE:
				break;
			case PRESS_DURATION_AND_DECAY:
			case PRESS_DURATION_ONLY:
				if (button->pressed_ticks < button->max_tick_count) { button->pressed_ticks += button->tick_growth_rate; }
				break;
			}
		}
		else {
			button->pressed = true;
		}
		button->released = false;
		break;
	case 0: //allegro uses 0 for false
		if (button->pressed) {
			//NOTE we must manaully clear a released state when were done with it
			//but we can due this also via rge_button_was_released(button_input_state_t* button)
			button->released = true;
		}

		//how many ticks to remove when the button is longer pressed
		switch (rge_input_state.mode) {
		case SIMPLE:
			break;
		case PRESS_DURATION_ONLY:
			button->pressed_ticks = 0;
			break;
		case PRESS_DURATION_AND_DECAY:
			if (button->pressed_ticks > 0) {
				button->pressed_ticks = (button->pressed_ticks - button->tick_decay_rate > 0) ? button->pressed_ticks - button->tick_decay_rate : 0;
			}
			break;
		}
		button->pressed = false;
		break;
	default:
#ifdef RGE_DEBUG
		assert("Unhandled input exception!  Check default block in rge_set_button_state!" && false);
#endif
		break;
	}
}

void rge_input() {

	//UP/DOWN/LEFT/RIGHT
	rge_set_button_state(&rge_input_state.up, key[DEFAULT_UP_KEY]);
	rge_set_button_state(&rge_input_state.left, key[DEFAULT_LEFT_KEY]);
	rge_set_button_state(&rge_input_state.right, key[DEFAULT_RIGHT_KEY]);
	rge_set_button_state(&rge_input_state.down, key[DEFAULT_DOWN_KEY]);

	//A/B
	rge_set_button_state(&rge_input_state.a, key[DEFAULT_A_KEY]);
	rge_set_button_state(&rge_input_state.b, key[DEFAULT_B_KEY]);

	//START/SELECT
	rge_set_button_state(&rge_input_state.start, key[DEFAULT_START_KEY]);
	rge_set_button_state(&rge_input_state.select, key[DEFAULT_SELECT_KEY]);

#ifdef RGE_EXTENDED_INPUT
	/*
	TODO: DEFINED but not implemented
	button_input_state_t kb_f1;
	button_input_state_t kb_f2;
	button_input_state_t kb_f3;
	button_input_state_t kb_f4;
	button_input_state_t kb_f5;
	button_input_state_t kb_f6;
	button_input_state_t kb_f7;
	button_input_state_t kb_f8;
	button_input_state_t kb_f9;
	button_input_state_t kb_f10;
	button_input_state_t kb_f11;
	button_input_state_t kb_f12;
	button_input_state_t kb_del;
	*/

	rge_set_button_state(&rge_input_state.kb_esc, key[KEY_ESC]);
	rge_set_button_state(&rge_input_state.kb_comma, key[KEY_COMMA]);
	rge_set_button_state(&rge_input_state.kb_enter, key[KEY_ENTER]);
	//key stop is '.', legacy callback in the API to old name
	rge_set_button_state(&rge_input_state.kb_period, key[KEY_STOP]); 
	rge_set_button_state(&rge_input_state.kb_semicolon, key[KEY_SEMICOLON]); 
	rge_set_button_state(&rge_input_state.kb_single_qoute, key[KEY_QUOTE]); 
	rge_set_button_state(&rge_input_state.kb_open_sqr_bracket, key[KEY_OPENBRACE]); 
	rge_set_button_state(&rge_input_state.kb_close_sqr_bracket, key[KEY_CLOSEBRACE]); 
	rge_set_button_state(&rge_input_state.kb_slash, key[KEY_SLASH]); 
	rge_set_button_state(&rge_input_state.kb_backslash, key[KEY_BACKSLASH]); 
	rge_set_button_state(&rge_input_state.kb_backspace, key[KEY_BACKSPACE]); 
	rge_set_button_state(&rge_input_state.kb_a, key[KEY_A]);
	rge_set_button_state(&rge_input_state.kb_b, key[KEY_B]);
	rge_set_button_state(&rge_input_state.kb_c, key[KEY_C]);
	rge_set_button_state(&rge_input_state.kb_d, key[KEY_D]);
	rge_set_button_state(&rge_input_state.kb_e, key[KEY_E]);
	rge_set_button_state(&rge_input_state.kb_f, key[KEY_F]);
	rge_set_button_state(&rge_input_state.kb_g, key[KEY_G]);
	rge_set_button_state(&rge_input_state.kb_h, key[KEY_H]);
	rge_set_button_state(&rge_input_state.kb_i, key[KEY_I]);
	rge_set_button_state(&rge_input_state.kb_j, key[KEY_J]);
	rge_set_button_state(&rge_input_state.kb_k, key[KEY_K]);
	rge_set_button_state(&rge_input_state.kb_l, key[KEY_L]);
	rge_set_button_state(&rge_input_state.kb_m, key[KEY_M]);
	rge_set_button_state(&rge_input_state.kb_n, key[KEY_N]);
	rge_set_button_state(&rge_input_state.kb_o, key[KEY_O]);
	rge_set_button_state(&rge_input_state.kb_p, key[KEY_P]);
	rge_set_button_state(&rge_input_state.kb_q, key[KEY_Q]);
	rge_set_button_state(&rge_input_state.kb_r, key[KEY_R]);
	rge_set_button_state(&rge_input_state.kb_s, key[KEY_S]);
	rge_set_button_state(&rge_input_state.kb_t, key[KEY_T]);
	rge_set_button_state(&rge_input_state.kb_u, key[KEY_U]);
	rge_set_button_state(&rge_input_state.kb_v, key[KEY_V]);
	rge_set_button_state(&rge_input_state.kb_w, key[KEY_W]);
	rge_set_button_state(&rge_input_state.kb_x, key[KEY_X]);
	rge_set_button_state(&rge_input_state.kb_y, key[KEY_Y]);
	rge_set_button_state(&rge_input_state.kb_z, key[KEY_Z]);
	rge_set_button_state(&rge_input_state.kb_0, key[KEY_0]);
	rge_set_button_state(&rge_input_state.kb_1, key[KEY_1]);
	rge_set_button_state(&rge_input_state.kb_2, key[KEY_2]);
	rge_set_button_state(&rge_input_state.kb_3, key[KEY_3]);
	rge_set_button_state(&rge_input_state.kb_4, key[KEY_4]);
	rge_set_button_state(&rge_input_state.kb_5, key[KEY_5]);
	rge_set_button_state(&rge_input_state.kb_6, key[KEY_6]);
	rge_set_button_state(&rge_input_state.kb_7, key[KEY_7]);
	rge_set_button_state(&rge_input_state.kb_8, key[KEY_8]);
	rge_set_button_state(&rge_input_state.kb_9, key[KEY_9]);
	rge_set_button_state(&rge_input_state.kb_up, key[KEY_UP]);
	rge_set_button_state(&rge_input_state.kb_down, key[KEY_DOWN]);
	rge_set_button_state(&rge_input_state.kb_left, key[KEY_LEFT]);
	rge_set_button_state(&rge_input_state.kb_right, key[KEY_RIGHT]);
	rge_set_button_state(&rge_input_state.kb_pgup, key[KEY_PGUP]);
	rge_set_button_state(&rge_input_state.kb_pgdn, key[KEY_PGDN]);
	rge_set_button_state(&rge_input_state.kb_tab, key[KEY_TAB]);
	rge_set_button_state(&rge_input_state.kb_space, key[KEY_SPACE]);
#endif

	//TODO:  Move shutdown code below into debug and provice single key shutdown ONLY in RGE_DEBUG
	//SHUTDOWN
	rge_set_button_state(&rge_input_state.shutdown_key, key[DEFAULT_SHUTDOWN_KEY]);

	//shutdown key pressed exit on next frame
	if (rge_button_was_released(&rge_input_state.shutdown_key)) {  }

#ifdef RGE_DEBUG
	rge_set_button_state(&rge_input_state.debug_toggle_key_state, key[TOGGLE_DEBUG_KEY]);
	rge_set_button_state(&rge_input_state.debug_next_msg_key_state, key[DEBUG_NEXT_MSG_KEY]);
	rge_set_button_state(&rge_input_state.debug_set_mode_key_state, key[DEBUG_SET_MODE_KEY]);  //toggle vram, palette, etc...
	
	if (rge_button_was_released(&rge_input_state.debug_toggle_key_state)) { rge_debug.visible = !rge_debug.visible; }
	switch (rge_debug.visible) {
	case true:
		if (rge_button_was_released(&rge_input_state.debug_next_msg_key_state)) { rge_debug.active_message_index++; }

		//we should just use a mod based solution here but meh for now it works
		if (rge_debug.active_message_index >= DEBUG_MSG_COUNT - 1) { rge_debug.active_message_index = 0; }

		//change mode from vram to palette, etc...
		if (rge_button_was_released(&rge_input_state.debug_set_mode_key_state)) { 
			switch (rge_debug.debug_mode) {
			case DEBUG_PALETTE:
				rge_debug.debug_mode = DEBUG_VRAM;
				break;
			case DEBUG_VRAM:
				rge_debug.debug_mode = DEBUG_NO_OVERLAY;
				break;
			case DEBUG_NO_OVERLAY:
				rge_debug.debug_mode = DEBUG_PALETTE;
				break;
			}
		}
		break;
	case false:
		break;
	}

#endif
}
#endif  //END __RGE_ENGINE_INPUT_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// SPRITES, TILES and TILEMAPS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_SPRITES_TILE_AND_TILEMAPS_SYSTEM_BLOCK

//returns byte with specified index flag set to value
unsigned char set_byte_flag(unsigned char byte, unsigned char index, bool value) {
	switch (value) {
	case true:
		//set bit
		return byte | 0b00000001 << (index % 8);
	case false:
		//unset bit
		return byte & ~(0b00000001 << (index % 8));
	}
}

//returns whether byte has index set (to 1) or not 
bool get_byte_flag(unsigned char byte, unsigned char index) {
	return byte & (0b00000001 << (index % 8));
}

//WHERE IS get_1bpp_pixel_color?  
/**
 * 1bpp basically just flags if a pixel is on or off
 * so we can just use set/get_byte_flag for this purpose and draw the pixel at a desired
 * color set globally
 */

 //retrieves pixel color for 2bpp index color sprite from byte
unsigned char get_2bpp_pixel_color(unsigned char byte, unsigned char pixel) {
	//each byte under 2bpp stores half a sprite row, so 4 pixels per byte
	switch (pixel % 4) {
	case 0:
		return (byte & 0b11000000) >> 6;
	case 1:
		return (byte & 0b00110000) >> 4;
	case 2:
		return (byte & 0b00001100) >> 2;
	case 3:
		return (byte & 0b00000011);
	}
}

//retrieves pixel color for 4bpp index color sprite from byte
unsigned char get_4bpp_pixel_color(unsigned char byte, unsigned char pixel) {
	//each byte under 4bpp stores 1/4 sprite row, or 2 pixels per byte
	switch (pixel % 2) {
	case 0://0xF0
		return (byte & 0b11110000) >> 4;
	case 1://0x0F
		return (byte & 0b00001111);
	}
}

bool convert_pixel_to_1bpp(BITMAP* layer, int px_x, int px_y) {
	//we take a pixel index of rge_gfx.clear_color to mean not set, any other
	//color value is interpretted as being set
	return (PGET(layer, px_x, px_y) != rge_gfx.clear_color);
}

//convert a bitmap sprite to sprite_1bpp_t type for export
sprite_1bpp_t convert_bmp_sprite_to_1bpp_sprite(int sprite_id, unsigned char flags) {
	
	sprite_1bpp_t sprite = {
		.flags = 0b00000000,
		.sprite = {
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		}
	};

	//grab sprite bitmap
	for (int px_y = 0; px_y < 8; px_y++) {
		for (int px_x = 0; px_x < 8; px_x++) {
			//is px_x set?  if yes, set in sprite_1bpp_t
		
			//get sprite row (so for 1bpp this is 8 rows)
			//we are upscaled to 16px from 8px so we need to double up on px_x,px_y
			bool is_px_set = convert_pixel_to_1bpp(sprites[sprite_id], px_x * 2, px_y * 2);
			sprite.sprite[px_y] = set_byte_flag(sprite.sprite[px_y], 7 - px_x, is_px_set);	
		}
	}

	sprite.flags = flags;

	return sprite;
}
#endif  //END __RGE_ENGINE_SPRITES_TILE_AND_TILEMAPS_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// AUDIO SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_AUDIO_SYSTEM_BLOCK

rge_state_t rge_audio_init() {
	
	//install a digital sound driver 
	if (install_sound(DIGI_AUTODETECT, MIDI_NONE, "") != 0) {
		assert("Error initializing sound system" && false);
		//well never read here with assert
		return RGE_STATE_AUDIO_INIT_FAILED;
	}
	else {
		return RGE_STATE_OK;
	}
}

void rge_audio_manager_add_sample(audio_sample_t sample)
{
	//check component name is not in use
	rge_string_t* sample_name = sample.name;
	audio_sample_t* tmp = rge_audio_manager_get_sample_by_name(sample_name->text);
	if (tmp) {
		assert("audio_sample_t filename provided is already in use" && false);
	}
	rge_audio_manager_update_indexes();
	arraystack_push(&rge_audio_manager.audio_samples, &sample);
	rge_audio_manager_update_indexes();
}


void rge_audio_manager_play_sample(char* name) {
	//check audio filename exists (is loaded)
	audio_sample_t* tmp = rge_audio_manager_get_sample_by_name(name);
	if (!tmp) {
		assert("audio_sample_t filename provided does not exist" && false);
	}
	play_sample(tmp->sample, tmp->volume, tmp->pan, tmp->pitch, tmp->loop);
}

void rge_audio_manager_stop_sample(char* name) {
	//check audio filename exists (is loaded)
	audio_sample_t* tmp = rge_audio_manager_get_sample_by_name(name);
	if (!tmp) {
		assert("audio_sample_t filename provided does not exist" && false);
	}
	stop_sample(tmp->sample);
}

void rge_audio_sample_init(audio_sample_t* sample, char* filename, char* sample_name, int pan, int pitch, int volume, bool loop) {
	sample->id = -1;  //-1 indicates unmanaged audio sample (ie: not managed by audio_manager_t instance)
	sample->name = new_string(sample_name);
	sample->loop = loop;
	sample->pan = pan;
	sample->pitch = pitch;
	sample->volume = volume;
	//attempt to load sample from disk
	sample->sample = load_sample(filename);
	if (!sample->sample) {
		assert("Unable to load provided audio sample!" && false);
	}
}

void rge_audio_manager_set_global_volume(int volume) {
	rge_audio_manager.global_volume = volume;
	set_volume(rge_audio_manager.global_volume, rge_audio_manager.global_volume);
	//this call doesn't appear to do anything on Windows 10
	//set_hardware_volume(rge_audio_manager.global_volume, rge_audio_manager.global_volume);
}

void rge_audio_manager_init() {

	rge_audio_manager.set_global_volume = rge_audio_manager_set_global_volume;
	rge_audio_manager.set_global_volume(255);
	rge_audio_manager.play_sample = rge_audio_manager_play_sample;
	rge_audio_manager.add_sample = rge_audio_manager_add_sample;
	rge_audio_manager.stop_sample = rge_audio_manager_stop_sample;
	rge_audio_manager.dispose_sample = rge_audio_manager_dispose_sample;
	rge_audio_manager.dispose = rge_audio_manager_dispose;

	//init arraystack
	arraystack_init(&rge_audio_manager.audio_samples, sizeof(audio_sample_t));

}

//make sure ui_component_t->props->id is the index within rge_ui_manager->components arraystack, for fast lookups
void rge_audio_manager_update_indexes() {
	if (rge_audio_manager.audio_samples.length > 0) {
		int index = 0;
		//update ids (indexes)
		audio_sample_t* tmp;
		for (int i = 0; i < rge_audio_manager.audio_samples.length; i++) {
			tmp = arraystack_get_pointer(&rge_audio_manager.audio_samples, i);
			tmp->id = index++;
		}
	}
}

audio_sample_t* rge_audio_manager_get_sample_by_name(char* name) {
	audio_sample_t tmp;
	for (int i = 0; i < rge_audio_manager.audio_samples.length; i++) {
		arraystack_get(&rge_audio_manager.audio_samples, i, &tmp);
		//check if names match
		rge_string_t* sample_name = tmp.name;
		if (strcmp(name, sample_name->text) == 0) {
			return arraystack_get_pointer(&rge_audio_manager.audio_samples, i);
		}
	}
	return NULL;
}

void rge_audio_manager_dispose_sample(int id)
{
	audio_sample_t tmp;
	int index_of_sample_id_to_remove = -1;  //index in arraystack of audio sample with 'id'

	for (int i = 0; i < rge_audio_manager.audio_samples.length; i++) {
		arraystack_get(&rge_audio_manager.audio_samples, i, &tmp);
		if (tmp.id == id) {
			index_of_sample_id_to_remove = i;
			break;
		}
	}

	if (index_of_sample_id_to_remove != -1) {
		arraystack_remove(&rge_audio_manager.audio_samples, index_of_sample_id_to_remove, &tmp);
		//free name
		dispose_string(tmp.name);
		//attempt to dispose of SAMPLE* 
		destroy_sample(tmp.sample);
	}
#ifdef RGE_DEBUG
	if (index_of_sample_id_to_remove == -1) {
		//TODO:  Problem appears to be (and UI/SCENES/SETTINGS should all be affected) is that the iterator doesn't update for removed components
		//so on subsequent calls we never reach i = desired index BECAUSE rge_audio_manager.collection.length will be less than the expected value
		assert("Attempt to remove non-existent audio sample index from rge_audio_manager, check your 'id' is valid." && false);
	}
#endif

	rge_audio_manager_update_indexes();

}

void rge_audio_manager_dispose() {

	//only create an iterator if we have samples in the manager
	if (rge_audio_manager.audio_samples.length > 0) {
		audio_sample_t tmp;
		while (rge_audio_manager.audio_samples.length > 0) {
			arraystack_get(&rge_audio_manager.audio_samples, 0, &tmp);
			rge_audio_manager_dispose_sample(tmp.id);
		}
	}
	arraystack_dispose(&rge_audio_manager.audio_samples);
}

#endif  //END __RGE_ENGINE_AUDIO_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// UI SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_UI_SYSTEM_BLOCK

//-----------------------------------------------------------------------------
// UI - COMPONENT
//-----------------------------------------------------------------------------
void rge_ui_component_init(ui_component_t* component, const char* name, const char* msg, const char* scene_name, ui_padding_t padding, int x, int y, int width, int height,
		color_palette_index_t foreground, color_palette_index_t background, color_palette_index_t border_highlight, 
	void (*draw_impl)(ui_component_props_t* props), void (*update_impl)(ui_component_props_t* props), char* callback_args, rge_ui_callback_func_t activation_callback) {
	component->props.name = new_string(name);
	component->props.msg = new_string(msg);
	if (scene_name != NULL) {
		component->props.scene_name = new_string(scene_name);
	} else {
		component->props.scene_name = new_string(RGE_GLOBAL_SCENE);
	}
	component->props.state = NORMAL;
	component->props.padding = padding;
	component->props.x = x;
	component->props.y = y;
	component->props.is_visible = true;
	component->props.width = width;
	component->props.height = height;
	component->props.background = background;
	component->props.foreground = foreground;
	component->props.border_highlight = border_highlight;
	component->props.id = -1; //-1 indicates that a unmanaged button is in use (unmannaged means its lifecycle isn't managed by rge_ui_manager)
	//if callback args is null, do nothing
	if (callback_args != NULL) {
		component->props.callback_args = new_string(callback_args);
	} else {
		component->props.callback_args = NULL;
	}
	component->props.activation_callback = activation_callback;
	component->draw = draw_impl;
	component->update = update_impl;
	component->dispose = rge_ui_component_dispose;
}
void rge_ui_component_init_auto_size(ui_component_t* component, const char* name, const char* msg, const char* scene_name, int x, int y,
	color_palette_index_t foreground, color_palette_index_t background, color_palette_index_t border_highlight,
	void (*draw_impl)(ui_component_props_t* props), void (*update_impl)(ui_component_props_t* props), char* callback_args, rge_ui_callback_func_t activation_callback) {

	ui_padding_t padding;
	//pad 2px on all sides of text
	padding.top = padding.bottom = padding.left = padding.right = 0;

	//automatically size component based on text size
	rge_ui_component_init(component, name, msg, scene_name, padding, x, y,
		strlen(msg) * UI_CHAR_WIDTH_PX, UI_CHAR_HEIGHT_PX,
		foreground, background, border_highlight, draw_impl, update_impl, callback_args, activation_callback);
}
rge_ui_callback_func_t rge_ui_component_no_callback() {
	assert("Attempt to call activation_callback on unsupported component type.  Either replace activation_callback explicitly or don't call this method.");
}

void rge_ui_component_dispose(ui_component_props_t* props) {
	dispose_string(props->name);
	dispose_string(props->msg);
	dispose_string(props->scene_name);
	if (props->callback_args) {
		dispose_string(props->callback_args);
	}
}

//-----------------------------------------------------------------------------
// UI - MULTILINE LABEL
//-----------------------------------------------------------------------------
void rge_ui_multiline_label_init(ui_multiline_label_t* label, const char* name, const char* msg, const char* scene_name, ui_padding_t padding, int x, int y, int width, int height,
	color_palette_index_t foreground, color_palette_index_t background, color_palette_index_t border_highlight) {
	//we auto-set height based on width and message size
	//break msg into strings that will fit in width of label
	int char_per_row = ceil(((float)width - (float)(padding.left + padding.right)) / (float)UI_CHAR_WIDTH_PX);
	int num_rows = ceil((float)(strlen(msg)) / (float)char_per_row);
	int adjusted_height = ((num_rows) * UI_CHAR_HEIGHT_PX) + padding.top + padding.bottom;

#ifdef RGE_DEBUG
	bool padding_horizontal_ok = (width > (padding.left + padding.right));
	//check padding is less than width
	assert("Left and right padding is greater than or equal to display width, reduce padding." && padding_horizontal_ok);
#endif

	rge_ui_component_init(&label->component, name, msg, scene_name, padding, x, y, width, adjusted_height, foreground, background, border_highlight, 
		rge_ui_multiline_label_draw, rge_ui_multiline_label_update, NULL, rge_ui_component_no_callback);
}
void rge_ui_multiline_label_update(ui_component_props_t* props) {
	//TODO:  Auto-size elements here when updating strings
}
void rge_ui_multiline_label_draw(ui_component_props_t* props) {
	rge_gfx.fill_color = props->background;
	rge_gfx.rect(ui_layer, props->x, props->y, props->width, props->height);
	rge_gfx.stroke_color = props->border_highlight;
	rge_gfx.rect_nofill(ui_layer, props->x - 1, props->y - 1, props->width + 1, props->height + 1);
	
	int char_per_row = ceil(((float)props->width - (float)(props->padding.left + props->padding.right)) / (float)UI_CHAR_WIDTH_PX);
	int char_index = 0;
	int num_rows = ceil((float)(props->msg->length - 1) / (float)char_per_row);

	//+2 here for \0
	char* line_buffer = malloc((char_per_row + 2) * sizeof(char));

	for (int i = 0, char_index = 0; i < num_rows; i++, char_index += char_per_row) {
		//grab the substring
		snprintf(line_buffer, char_per_row + 1, "%s", props->msg->text + char_index);
		rge_gfx.text(ui_layer, props->x + props->padding.left, props->y + props->padding.top + (i*UI_CHAR_HEIGHT_PX), line_buffer, props->foreground, props->background, TEXT_ALIGN_LEFT);
	}

	//free line_buffer
	free(line_buffer);
}

//-----------------------------------------------------------------------------
// UI - BUTTON
//-----------------------------------------------------------------------------
void rge_ui_button_init(ui_button_t* button, const char* name, const char* msg, const char* scene_name, int x, int y, int width, int height,
	color_palette_index_t foreground, color_palette_index_t background, color_palette_index_t border_highlight, char* callback_args, rge_ui_callback_func_t activation_callback) {
	rge_ui_component_init_auto_size(&button->component, name, msg, scene_name, x, y, foreground, background, border_highlight, rge_ui_button_draw, rge_ui_button_update, callback_args, activation_callback);
}

void rge_ui_button_update(ui_component_props_t* props) {
	switch (props->state) {
	case ACTIVATED:
		props->state = HIGHLIGHTED;  //revert to highlighted
		//we have callback argumetns, send them along
		if (props->activation_callback) {
			if (props->callback_args != NULL) {
				rge_string_t* callback_args = props->callback_args;
				props->activation_callback(callback_args->text);
			}
			else {
				props->activation_callback(NULL);
			}
		}

	}
}

void rge_ui_button_draw(ui_component_props_t* props) {
	//TODO:  Make rge_ui_button_draw use padding
	switch (props->state) {
	case NORMAL:
		//draw background
		rge_gfx.fill_color = props->foreground;
		rge_gfx.rect(ui_layer, props->x - props->width/2 + 1, props->y + 1, props->width + 1, props->height);
		//draw text
		rge_gfx.text(ui_layer, props->x, props->y, props->msg->text, props->foreground, props->background, TEXT_ALIGN_CENTER);
		break;
	case HIGHLIGHTED:
		//draw background
		rge_gfx.fill_color = props->foreground;
		rge_gfx.rect(ui_layer, props->x - props->width / 2 + 1, props->y + 1, props->width + 1, props->height);
		//draw text
		rge_gfx.text(ui_layer, props->x, props->y, props->msg->text, props->foreground, props->background, TEXT_ALIGN_CENTER);
		rge_gfx.stroke_color = props->border_highlight;
		rge_gfx.rect_nofill(ui_layer, props->x - props->width / 2 - 1, props->y - 1, props->width + 1, props->height + 1);
		break;
	case ACTIVATED:
		//draw background
		rge_gfx.fill_color = props->background;
		rge_gfx.rect(ui_layer, props->x - props->width / 2 + 3, props->y + 3, props->width + 1, props->height);
		//draw text
		rge_gfx.text(ui_layer, props->x + 2, props->y + 2, props->msg->text, props->background, props->foreground, TEXT_ALIGN_CENTER);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// UI MANAGER
///////////////////////////////////////////////////////////////////////////////
void rge_ui_manager_init() {
	rge_ui_manager.add_component = rge_ui_manager_add_component;
	rge_ui_manager.dispose_component = rge_ui_manager_dispose_component;

	rge_ui_manager.draw = rge_ui_manager_draw;
	rge_ui_manager.update = rge_ui_manager_update;
	rge_ui_manager.dispose = rge_ui_manager_dispose;

	//init arraystack
	arraystack_init(&rge_ui_manager.components, sizeof(ui_component_t));
}

void rge_ui_manager_add_component(ui_component_t component) {
	//check component name is not in use
	rge_string_t* component_name = component.props.name;
	ui_component_t* tmp = rge_ui_manager_get_component_by_name(component_name->text);
	if (tmp) {
		assert("ui_component_t name provided is already in use" && false);
	}
	arraystack_push(&rge_ui_manager.components, &component);
	rge_ui_manager_update_indexes();
}

//make sure ui_component_t->props->id is the index within rge_ui_manager->components arraystack, for fast lookups
void rge_ui_manager_update_indexes() {
	if (rge_ui_manager.components.length > 0) {
		int index = 0;
		//update ids (indexes)
		ui_component_t* tmp;
		for (int i = 0; i < rge_ui_manager.components.length; i++) {
			tmp = arraystack_get_pointer(&rge_ui_manager.components, i);
			tmp->props.id = index++;
		}
	}
}

void rge_ui_manager_dispose_component(int id) {
	ui_component_t tmp;
	int index_of_component_id_to_remove = -1;  //index in arraystack of scene with 'id'

	for (int i = 0; i < rge_ui_manager.components.length; i++) {
		arraystack_get(&rge_ui_manager.components, i, &tmp);
		if (tmp.props.id == id) {
			index_of_component_id_to_remove = i;
			break;
		}
	}

	if (index_of_component_id_to_remove != -1) {
		arraystack_remove(&rge_ui_manager.components, index_of_component_id_to_remove, &tmp);
		//dispose of scene
		tmp.dispose(&tmp.props);
	}
#ifdef RGE_DEBUG
	if (index_of_component_id_to_remove == -1) {
		assert("Attempt to remove non-existent component index from rge_ui_manager, check your 'id' is valid." && false);
	}
#endif

	rge_ui_manager_update_indexes();
}

ui_component_t* rge_ui_manager_get_component_by_name(char* name) {
	ui_component_t tmp;
	for (int i = 0; i < rge_ui_manager.components.length; i++) {
		arraystack_get(&rge_ui_manager.components, i, &tmp);
		//check if names match
		rge_string_t* component_name = tmp.props.name;
		if (strcmp(name, component_name->text) == 0) {
			return arraystack_get_pointer(&rge_ui_manager.components, i);
		}
	}
	return NULL;
}

void rge_ui_manager_update() {
	ui_component_t tmp;
	for (int i = 0; i < rge_ui_manager.components.length; i++) {
		arraystack_get(&rge_ui_manager.components, i, &tmp);
		tmp.update(&tmp.props);
	}
}

void rge_ui_manager_draw() {
	//TODO: optimize here, arraystack_get does a memcopy but we could
	//just use direct pointer access
	ui_component_t tmp;
	for (int i = 0; i < rge_ui_manager.components.length; i++) {
		arraystack_get(&rge_ui_manager.components, i, &tmp);

		//only draw global ui components, these are components not associated with a scene
		if (tmp.props.is_visible && (strcmp(tmp.props.scene_name->text, RGE_GLOBAL_SCENE) == 0)) {
			tmp.draw(&tmp.props);
		}
#ifdef RGE_DEBUG
		char debug_info[32];
		snprintf(debug_info, 32, "->ID: %d", tmp.props.id);
		rge_draw_text(debug_layer, tmp.props.x + tmp.props.width/2, tmp.props.y, debug_info, 255, 254, TEXT_ALIGN_LEFT);
#endif
	}
}

void rge_ui_manager_dispose() {
	
	//only create an iterator if we have components in the manager
	if (rge_ui_manager.components.length > 0) {
		ui_component_t tmp;
		//delete each ui  
		while (rge_ui_manager.components.length > 0) {
			arraystack_get(&rge_ui_manager, 0, &tmp);
			tmp.dispose(&tmp.props);
		}

	}
	arraystack_dispose(&rge_ui_manager.components);
}
#endif  //END __RGE_ENGINE_UI_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// TIMER SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_TIMER_SYSTEM_BLOCK
int rge_millis_since_start() {
	return global_timer_tick;
}

int rge_seconds_since_start() {
	return global_timer_tick / 1000;
}

int rge_minutes_since_start() {
	return global_timer_tick / 60000;
}
#endif //__RGE_ENGINE_TIMER_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// SCENE SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_SCENE_SYSTEM_BLOCK
//-----------------------------------------------------------------------------
// SCENE
//-----------------------------------------------------------------------------
void rge_scene_init(scene_t* scene, char* name, rge_scene_void_func_t draw, rge_scene_void_func_t update, rge_scene_void_func_t dispose) {
	scene->id = -1;  //-1 indicates unmanaged scene (ie: not managed by scene_manager_t)
	scene->name = new_string(name);
	scene->is_visible = true;
	scene->is_active = true;
	//init settings
	rge_settings_init(&scene->settings);
	scene->draw = draw;
	scene->update = update;
	scene->dispose = dispose;
	scene->init_complete = true;
}

///////////////////////////////////////////////////////////////////////////////
// SCENE MANAGER
///////////////////////////////////////////////////////////////////////////////
void rge_scene_manager_add_scene(scene_t scene)
{
	//check component name is not in use
	rge_string_t* scene_name = scene.name;
	scene_t* tmp = rge_scene_manager_get_scene_by_name(scene_name->text);
	if (tmp) {
		assert("scene_t name provided is already in use" && false);
	}

	arraystack_push(&rge_scene_manager.scenes, &scene);
	rge_scene_manager_update_indexes();
	//TODO: add check here that active_scene actually exists
	if (rge_scene_manager.active_scene) {
		//disable scene that we are going to overlay
		rge_scene_manager_get_scene_by_name(rge_scene_manager.active_scene)->is_active = false;
	}

	rge_scene_manager.active_scene = scene_name->text;
}

void rge_scene_manager_init() {
	rge_scene_manager.add_scene = rge_scene_manager_add_scene;
	rge_scene_manager.dispose_scene = rge_scene_manager_dispose_scene;

	rge_scene_manager.draw = rge_scene_manager_draw;
	rge_scene_manager.update = rge_scene_manager_update;
	rge_scene_manager.dispose = rge_scene_manager_dispose;

	//init arraystack
	arraystack_init(&rge_scene_manager.scenes, sizeof(scene_t));
	
}

//make sure ui_component_t->props->id is the index within rge_ui_manager->components arraystack, for fast lookups
void rge_scene_manager_update_indexes() {
	if (rge_scene_manager.scenes.length > 0) {
		int index = 0;
		//update ids (indexes)
		scene_t* tmp;
		for (int i = 0; i < rge_scene_manager.scenes.length; i++) {
			tmp = arraystack_get_pointer(&rge_scene_manager.scenes, i);
			tmp->id = index++;
		}
	}
}

scene_t* rge_scene_manager_get_scene_by_name(char* name) {
	scene_t tmp;
	for (int i = 0; i < rge_scene_manager.scenes.length; i++) {
		arraystack_get(&rge_scene_manager.scenes, i, &tmp);
		//check if names match
		rge_string_t* scene_name = tmp.name;
		if (strcmp(name, scene_name->text) == 0) {
			return arraystack_get_pointer(&rge_scene_manager.scenes, i);
		}
	}
	return NULL;
}

void rge_scene_manager_draw() {
	scene_t tmp;
	for (int i = 0; i < rge_scene_manager.scenes.length; i++) {
		arraystack_get(&rge_scene_manager.scenes, i, &tmp);
#ifdef RGE_DEBUG
		char debug_info[32];
		snprintf(debug_info, 32, "Scene: %s, ID: %d", tmp.name->text, tmp.id);
		rge_draw_text(debug_layer, 20, 20, debug_info, 255, 254, TEXT_ALIGN_LEFT);
#endif

		if (tmp.is_visible) {
			//draw scene
			tmp.draw();

			//draw scene ui components
			ui_component_t tmp_ui;
			for (int i = 0; i < rge_ui_manager.components.length; i++) {
				arraystack_get(&rge_ui_manager.components, i, &tmp_ui);

				//only draw ui components from this scene
				if (tmp_ui.props.is_visible && (strcmp(tmp_ui.props.scene_name->text, tmp.name->text) == 0)) {
					tmp_ui.draw(&tmp_ui.props);
				}
#ifdef RGE_DEBUG
				char debug_info[32];
				snprintf(debug_info, 32, "->ID: %d", tmp_ui.props.id);
				rge_draw_text(debug_layer, tmp_ui.props.x + tmp_ui.props.width / 2, tmp_ui.props.y, debug_info, 255, 254, TEXT_ALIGN_LEFT);
#endif
			}

		}

		
	}
}

void rge_scene_manager_update() {
	scene_t tmp;
	for (int i = 0; i < rge_scene_manager.scenes.length; i++) {
		arraystack_get(&rge_scene_manager.scenes, i, &tmp);
		
		if (tmp.is_active) {
			tmp.update();
		}
	}
}

void rge_scene_manager_dispose_scene(int id)
{
	scene_t tmp;
	int index_of_scene_id_to_remove = -1;  //index in arraystack of scene with 'id'
	
	for (int i = 0; i < rge_scene_manager.scenes.length; i++) {
		arraystack_get(&rge_scene_manager.scenes, i, &tmp);
		if (tmp.id == id) {
			index_of_scene_id_to_remove = i;
			break;
		}
	}
	
	if (index_of_scene_id_to_remove != -1) {
		
		arraystack_remove(&rge_scene_manager.scenes, index_of_scene_id_to_remove, &tmp);
		
		//check if the scene were removing is the active scene, if it is set the end of the arraylist 
		//ie: whats being rendered on top of everything as the active scene
		if (rge_scene_manager.active_scene && (strcmp(rge_scene_manager.active_scene, tmp.name->text) == 0)) {
			if (rge_scene_manager.scenes.length > 0) {
				scene_t* active_scene = arraystack_get_pointer(&rge_scene_manager.scenes, rge_scene_manager.scenes.length - 1);
				rge_scene_manager.active_scene = active_scene->name->text;
			} else {
				rge_scene_manager.active_scene = NULL;
			}
		}
		
		tmp.is_active = false;
		tmp.is_visible = false;

		//dispose of scene (user provided)
		tmp.dispose();
		//free name
		dispose_string(tmp.name);
		//free settings
		rge_settings_dispose(&tmp.settings);
	}
#ifdef RGE_DEBUG
	if (index_of_scene_id_to_remove == -1) {
		assert("Attempt to remove non-existent scene index from rge_scene_manager, check your 'id' is valid." && false);
	}
#endif

	rge_scene_manager_update_indexes();
}

void rge_scene_manager_dispose() {

	//only create an iterator if we have scenes in the manager
	if (rge_scene_manager.scenes.length > 0) {
		scene_t tmp;
		//delete each scene 
		while (rge_scene_manager.scenes.length > 0) {
			arraystack_get(&rge_scene_manager.scenes, 0, &tmp);
			rge_scene_manager_dispose_scene(tmp.id);
		}
	}
	arraystack_dispose(&rge_scene_manager.scenes);
}
#endif  //END __RGE_ENGINE_SCENE_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// FILESYSTEM, IMPORT/EXPORT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_FILE_SYSTEM_BLOCK

//import a sag file given specified format (1BPP,2BPP,4BPP) and a destination sprites array that 
//has enough room for sprites (256*format size)
//No error checking here so you should now what your loading or behaviour will be unexpected
sag_file_status_t import_sag_file(sprite_palette_format_t format, const char* filename, sag_filetype_t filetype) {
	sag_file_status_t import_status = FILE_ERROR_UNKNOWN_NOT_IMPORTED;
	
	FILE* in_fp;
	in_fp = fopen(filename, "rb");

	if (in_fp == NULL) {
		import_status = FILE_ERROR_UNKNOWN_NOT_IMPORTED;
	} else {
		switch (filetype) {
		case SAG:
			if (format == FORMAT_1BPP) {
				sprite_1bpp_t sprites[256];
				fread(sprites, sizeof(sprite_1bpp_t), 256, in_fp);

				//now draw each sprite into VRAM
				//TODO: when we figure out what we intend to use VRAM for (if any)
				//just copy directly in to sprites[] or vice versa (remove sprites in favour of vram instance)
				for (int sprite_id = 0; sprite_id < 256; sprite_id++) {
					int vram_x = ((sprite_id % 16) * 8);
					int vram_y = ((sprite_id / 16) * 8);
					rge_draw_1bpp_sprite(vram, sprites[sprite_id], vram_x, vram_y);

					//copy over sprite flags
					flags_for_sprites[sprite_id] = sprites[sprite_id].flags;
				}
				import_status = FILE_IMPORTED;
			}
			else if (format == FORMAT_2BPP) {
				//sprite_2bpp_t sprites[256];
				//fread(sprites, sizeof(sprite_2bpp_t), 256, in_fp);
				import_status = FILE_ERROR_UNKNOWN_NOT_IMPORTED;
			}
			else if (format == FORMAT_4BPP) {
				//sprite_4bpp_t sprites[256];
				//fread(sprites, sizeof(sprite_4bpp_t), 256, in_fp);
				import_status = FILE_ERROR_UNKNOWN_NOT_IMPORTED;
			}
			break;
		default:
			assert("Unsupported input filetype!" && false);
			break;
		}

		//copy vram to individual sprites
		for (int i = 0; i < 256; i++) {
			//create our sprites bitmaps
			sprites[i] = create_bitmap(16, 16);

			//get column
			int sprite_column = i % 16;
			//get row
			int sprite_row = i / 16;

			//copy from vram sheet into sprites
			stretch_blit(vram, sprites[i], sprite_column * 8, sprite_row * 8, 8, 8, 0, 0, 16, 16);
		}

		fclose(in_fp);

		return import_status;
	}	
}

//export a sag file given specified format (1BPP,2BPP,4BPP) and a destination sprites array that 
//has exactly 256 sprites @ format size
//No error checking here so you should now what your loading or behaviour will be unexpected 
sag_file_status_t export_sag_file(sprite_palette_format_t format, const char* filename, sag_filetype_t filetype) {

	sag_file_status_t export_status = FILE_ERROR_UNKNOWN_NOT_EXPORTED;

	if (rge_file_exists(filename)) {
		return FILE_EXISTS_NOT_EXPORTED;
	}

	FILE* out_fp;
	out_fp = fopen(filename, "wb");

	if (out_fp == NULL) {
		export_status = FILE_ERROR_UNKNOWN_NOT_EXPORTED;
	}
	else {
		switch (filetype) {
		case SAG:
			if (format == FORMAT_1BPP) {
				sprite_1bpp_t sprites[256];
				
				//convert sprites instance to sprite_1bpp_t
				for (int sprite_id = 0; sprite_id < 256; sprite_id++) {
					sprites[sprite_id] = convert_bmp_sprite_to_1bpp_sprite(sprite_id,
						flags_for_sprites[sprite_id]);
				}

				fwrite(sprites, sizeof(sprite_1bpp_t), 256, out_fp);

				export_status = FILE_EXPORTED;
			}
			else if (format == FORMAT_2BPP) {
				//sprite_2bpp_t sprites[256];
				//fread(sprites, sizeof(sprite_2bpp_t), 256, in_fp);
				export_status = FILE_ERROR_UNKNOWN_NOT_EXPORTED;
			}
			else if (format == FORMAT_4BPP) {
				//sprite_4bpp_t sprites[256];
				//fread(sprites, sizeof(sprite_4bpp_t), 256, in_fp);
				export_status = FILE_ERROR_UNKNOWN_NOT_EXPORTED;
			}
			break;
		default:
			export_status = FILE_ERROR_UNKNOWN_NOT_EXPORTED;
			break;
		}
		fclose(out_fp);
	}
}
#endif  //END __RGE_ENGINE_FILE_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// UNIT/MODULE TESTING (Calls built in tests to verify code is working)
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_UNIT_TESTING_BLOCK

#endif  //END __RGE_ENGINE_UNIT_TESTING_BLOCK
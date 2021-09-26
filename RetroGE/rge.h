#ifndef __2D_RGE
#define __2D_RGE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <allegro.h>
#include <winalleg.h>
#include <process.h>
#include "rge_palettes.h"
#include "rge_string.h" 
#include "rge_utils.h"
#include "ods_arraystack.h"
#include "ods_iterator.h"

///////////////////////////////////////////////////////////////////////////////
// RGE API (Macro methods for working with engine)
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_API_BLOCK

//PRIMITIVE DRAWING FUNCTIONS
#define RECT(layer,x,y,w,h) rge_draw_rect_no_fill(layer,x,y,w,h)
#define RECTFILL(layer,x,y,w,h) rge_draw_rect(layer,x,y,w,h)
#define CIRC(layer,x,y,r) rge_draw_circle_no_fill(layer,x,y,r)
#define CIRCFILL(layer,x,y,r) rge_draw_circle(layer,x,y,r)
#define LINE(layer,x1,y1,x2,y2) rge_draw_line(layer,x1,y1,x2,y2)
#define TRI(layer,x1,y1,x2,y2,x3,y3) rge_draw_triangle(layer,x1,y1,x2,y2,x3,y3)
#define TRIFILL(layer,x1,y1,x2,y2,x3,y3) rge_draw_triangle_no_fill(layer,x1,y1,x2,y2,x3,y3)
#define TEXT(layer,x,y,msg,fg,bg,align) rge_draw_text(layer,x,y,msg,fg,bg,align)

//PIXEL DRAWING FUNCTIONS
#define PSET(layer,x,y,color_id) rge_set_pixel(layer,x,y,color_id)
#define PGET(layer,x,y) rge_get_pixel(layer,x,y)

//PALETTE and COLOR MANIP. FUNCTIONS
#define STROKE(color_id) rge_gfx.stroke_color = color_id
#define FILL(color_id) rge_gfx.fill_color = color_id

//TIMING FUNCTIONS
#define MILLIS() rge_millis_since_start()
#define SECONDS() rge_seconds_since_start()
#define MINUTES() rge_minutes_since_start()

//SPRITE FUNCTIONS
#define SPR(layer,x,y,sprite_id,h_flip,v_flip) rge_draw_sprite(layer,x,y,sprite_id,h_flip,v_flip)
#define SSPR(layer,sx,sy,sw,sh,dx,dy,dw,dh,sprite_id) rge_draw_scaled_sprite(layer,sx,sy,sw,sh,dx,dy,dw,dh,sprite_id)
#define SPRROT(src, dest, x, y, clockwise) rge_draw_rotated_sprite(src, dest, x, y, clockwise)

//MATH AND RANDOM FUNCTIONS
//return an integer from 0 to max (inclusive)
#define RAND(max) rge_rand_int_from_range(max)

//TODO:  integrate array shuffle from rge_utils.h

//MUSIC and SFX FUNCTIONS

//MAP FUNCTIONS

//CAMERA FUNCTIONS
#define CLS() rge_clear()

//INPUT FUNCTIONS

#endif  //END __RGE_API_BLOCK

///////////////////////////////////////////////////////////////////////////////
// ENGINE CONFIGURATION AND DEFINES
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_CONFIG_BLOCK

#ifdef RGE_DEBUG
	//VS2019
	#define _CRTDBG_MAP_ALLOC  //report file where any detected memory leak is
	#include <crtdbg.h> //watch for memory leaks, report on exit
#endif

#define INPUT_MS 50 //1000MS / 20 CPS = CALLS PER SECOND
#define ONE_SECOND_MS 1000 //1000MS
#define REDRAW_MS ONE_SECOND_MS / 60 //1000MS / 60 FPS
#define GLOBAL_TIMER_TICKS_MS 1 //1MS so it will fire (hopefully 1000 times per second)

//ALL RGE graphics 8 bit paletted
//meaning 24 bit bitmap support
//or 2^8:r, 2^8:g, 2^8:b (3X8 BITS = 24 BIT)
#define RGE_COLOR_DEPTH 8

//under RGE all tiles are 8x8 and mapped into a 320x240 screen space
//which is the default resolution/smallest resolution
//we then scaled mask blit up to registered resolution @ run time
#define RGE_TILE_SIZE_W 8
#define RGE_TILE_SIZE_H 8
#define RGE_BASE_SCREEN_WIDTH 320
#define RGE_BASE_SCREEN_HEIGHT 240

#define RGE_BACKGROUND_MAP_TILE_X 32
#define RGE_BACKGROUND_MAP_TILE_Y 32

//GLOBAL KEYBINDINGS
#ifdef RGE_DEBUG
	#define TOGGLE_DEBUG_KEY KEY_TILDE
	#define DEBUG_NEXT_MSG_KEY KEY_PGUP
	#define DEBUG_SET_MODE_KEY KEY_HOME
	#define DEBUG_MSG_COUNT 5  //5 messages @ max of 40 characters
	#define DEBUG_MSG_SIZE 40 //max size of debug message is 8px per character / 320 screen size	
	#define DEBUG_MODE_OVERLAY_WIDTH 320 
	#define DEBUG_MODE_OVERLAY_HEIGHT 208	
	#define DEBUG_MODE_TEXT_AREA_HEIGHT 30
	#define DEBUG_MODE_TEXT_AREA_START_Y 210
	#define DEBUG_TILE_SIZE 8
#endif

#define DEFAULT_UP_KEY KEY_W
#define DEFAULT_LEFT_KEY KEY_A
#define DEFAULT_DOWN_KEY KEY_S
#define DEFAULT_RIGHT_KEY KEY_D
#define DEFAULT_A_KEY KEY_J
#define DEFAULT_B_KEY KEY_N
#define DEFAULT_START_KEY KEY_ENTER
#define DEFAULT_SELECT_KEY KEY_BACKSLASH
#define DEFAULT_SHUTDOWN_KEY KEY_ESC

#define UI_CHAR_WIDTH_PX 8
#define UI_CHAR_HEIGHT_PX 8

#define RGE_GLOBAL_SCENE "RGE_GLOBAL_SCENE"

#endif  //END __RGE_ENGINE_CONFIG_BLOCK

///////////////////////////////////////////////////////////////////////////////
// ENGINE STATE AND SIGNED/UNSINGED TYPE ALIAS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_STATE_AND_TYPES_BLOCK
//SIGNED AND UNSIGNED TYPES
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

//ENGINE STATE/ERRORS
typedef enum rge_state {
	//ALL IS WELL (for now)
	RGE_STATE_OK,
	//GRAPHICS
	RGE_STATE_AL4_INIT_FAILED,
	RGE_STATE_GFX_INIT_FAILED,
	//INPUT
	RGE_STATE_INPUT_INIT_FAILED,
	//TIMER SETUP
	RGE_STATE_TIMER_INIT_FAILED,
	RGE_STATE_FRAME_TIMER_INIT_FAILED,
	RGE_STATE_INPUT_TIMER_INIT_FAILED,
	RGE_STATE_AUDIO_INIT_FAILED,
	RGE_STATE_DEBUG_TIMER_INIT_FAILED
} rge_state_t;
#endif  //END __RGE_ENGINE_STATE_AND_TYPES_BLOCK 

///////////////////////////////////////////////////////////////////////////////
// SAVE AND SETTINGS SYSTEMS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_SAVE_AND_SETTINGS_BLOCK
//-----------------------------------------------------------------------------
// SETTINGS
//-----------------------------------------------------------------------------
typedef enum rge_settings_type {
	RGE_INT,
	RGE_BOOL,
	RGE_STRING
} rge_settings_type_t;

typedef struct rge_setting {
	rge_settings_type_t type;
	rge_string_t* key;
	rge_string_t* value;
} rge_setting_t;
void rge_setting_init(rge_setting_t* setting, rge_settings_type_t type, char* key, char* value);
void rge_setting_update(rge_setting_t* setting, char* new_value);
void rge_setting_dispose(rge_setting_t* setting);

typedef struct rge_settings {
	arraystack_t settings;
	iterator_t iterator;
	void (*add_setting)(struct rge_settings* settings, rge_settings_type_t type, char* key, char* value);
	void (*update_setting)(rge_setting_t* setting, char* key, char* new_value);
	void (*dispose_setting)(struct rge_settings* settings, char* key);
	int (*get_int_setting)(struct rge_settings* settings, char* key);
	bool (*get_bool_setting)(struct rge_settings* settings, char* key);
	char* (*get_string_setting)(struct rge_settings* settings, char* key);
} rge_settings_t;
void rge_settings_init(rge_settings_t* settings);
void rge_settings_add_setting(struct rge_settings* settings, rge_settings_type_t type, char* key, char* value);
void rge_settings_update_setting(struct rge_settings* settings, char* key, char* new_value);
void rge_settings_dipose_setting(struct rge_settings* settings, char* key);
bool rge_settings_is_key_in_use(struct rge_settings* settings, char* key);
int  rge_settings_get_int_setting(struct rge_settings* settings, char* key);
bool rge_settings_get_bool_setting(struct rge_settings* settings, char* key);
char* rge_settings_get_string_setting(struct rge_settings* settings, char* key);
void rge_settings_dispose(rge_settings_t* settings);
#endif  //END __RGE_ENGINE_SAVE_AND_SETTINGS_BLOCK

///////////////////////////////////////////////////////////////////////////////
// SPRITES, TILES and TILEMAPS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_SPRITES_TILE_AND_TILEMAPS_SYSTEM_BLOCK

//enum for determining indexed color format for sprites
typedef enum sprite_palette_format {
	FORMAT_1BPP = 8,
	FORMAT_2BPP = 16,
	FORMAT_4BPP = 32
} sprite_palette_format_t;

//sprite with 1-color support
typedef struct sprite_1bpp {
	//0-7 bit flags
	unsigned char flags;
	/*  The following would produce a X with active color
		1 0 0 0 | 0 0 0 1
		0 1 0 0 | 0 0 1 0
		0 0 1 0 | 0 1 0 0
		0 0 0 1 | 1 0 0 0
		-----------------
		0 0 0 1 | 1 0 0 0
		0 0 1 0 | 0 1 0 0
		0 1 0 0 | 0 0 1 0
		1 0 0 0 | 0 0 0 1
	*/
	//1 bit per pixel * 8 pixels = 8 bits per row * 8 rows (8 bytes)	
	unsigned char sprite[8];
} sprite_1bpp_t;

//WHERE IS 3BPP?  In my opinion 3BPP just complicates working with a single byte as
//you need to provide the full 3 bytes (24 bits) to propely mask out colors
//instead to keep with the nice multiples of 2,4 and 1 bits per pixel we just simulate 
//3BPP using a 4BPP structure which has the added benefit of allowing us to pack
//an alternate palette for indexes 0-7, 8-15

//sprite with 4-color support
typedef struct sprite_2bpp {
	unsigned char flags;
	/*  The following would produce a X with palette color 01 (1)
		01 00 00 00 | 00 00 00 01
		00 01 00 00 | 00 00 01 00
		00 00 01 00 | 00 01 00 00
		00 00 00 01 | 01 00 00 00
		-------------------------
		00 00 00 01 | 01 00 00 00
		00 00 01 00 | 00 01 00 00
		00 01 00 00 | 00 00 01 00
		01 00 00 00 | 00 00 00 01
	*/
	//2 bits per pixel * 8 pixels = 16 bits per row * 8 rows (16 bytes)	
	unsigned char sprite[16];
} sprite_2bpp_t;

//sprite with 16-color support
typedef struct sprite_4bpp {
	unsigned char flags;
	/*  The following would produce a X with palette color 0001 (1)
		0001 0000 0000 0000 | 0000 0000 0000 0001
		0000 0001 0000 0000 | 0000 0000 0001 0000
		0000 0000 0001 0000 | 0000 0001 0000 0000
		0000 0000 0000 0001 | 0001 0000 0000 0000
		-----------------------------------------
		0000 0000 0000 0001 | 0001 0000 0000 0000
		0000 0000 0001 0000 | 0000 0001 0000 0000
		0000 0001 0000 0000 | 0000 0000 0001 0000
		0001 0000 0000 0000 | 0000 0000 0000 0001

	*/
	//4 bits per pixel * 8 pixels = 32 bits (4 bytes) per row * 8 rows (32 bytes)	
	unsigned char sprite[32];
} sprite_4bpp_t;

//returns byte with specified index flag set to value
unsigned char set_byte_flag(unsigned char byte, unsigned char index, bool value);

//returns whether byte has index set (to 1) or not 
bool get_byte_flag(unsigned char byte, unsigned char index);

//WHERE IS get_1bpp_pixel_color?  
/**
 * 1bpp basically just flags if a pixel is on or off
 * so we can just use set/get_byte_flag for this purpose and draw the pixel at a desired
 * color set globally
 */

 //retrieves pixel color for 2bpp index color sprite from byte
unsigned char get_2bpp_pixel_color(unsigned char byte, unsigned char pixel);

//retrieves pixel color for 4bpp index color sprite from byte
unsigned char get_4bpp_pixel_color(unsigned char byte, unsigned char pixel);

unsigned char convert_pixel_to_1bpp(int px_x, int px_y);

//convert a bitmap sprite to sprite_1bpp_t type for export
sprite_1bpp_t convert_bmp_sprite_to_1bpp_sprite(int sprite_id);

#endif  //END __RGE_ENGINE_SPRITES_TILE_AND_TILEMAPS_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// GRAPHICS SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_GRAPHICS_SYSTEM_BLOCK
//index into PALETTE object (0-255), platform dependent
typedef u8 color_palette_index_t;

//TODO move this somewhere else
typedef enum text_align {
	TEXT_ALIGN_LEFT,
	TEXT_ALIGN_CENTER,
	TEXT_ALIGN_RIGHT
} text_align_t;

typedef enum screen_resolution {
	RESOLUTION_320x240,
	RESOLUTION_640x480,
	RESOLUTION_1280x960
} screen_resolution_t;

typedef struct graphics {
	u16 screen_width;
	u16 screen_height;
	u16 game_width;
	u16 game_height;
	color_palette_index_t clear_color;
	color_palette_index_t stroke_color;
	color_palette_index_t fill_color;
	void (*text)(BITMAP* layer, int x, int y, const char* message, color_palette_index_t foreground, color_palette_index_t background, text_align_t text_alignment);
	void (*ellipse)(BITMAP* layer, int x, int y, int radius);
	void (*triangle)(BITMAP* layer, int x1, int y1, int x2, int y2, int x3, int y3);
	void (*rect)(BITMAP* layer, int x, int y, int width, int height);
	void (*rect_nofill)(BITMAP* layer, int x, int y, int width, int height);
	void (*flip_backbuffer)();
	void (*clear)(void);
} graphics_t;

void rge_draw_text(BITMAP* layer, int x, int y, const char* message, color_palette_index_t foreground, color_palette_index_t background, text_align_t text_alignment);
void rge_flip_backbuffer();
void rge_clear();
void rge_draw_sprite(BITMAP* layer, int sprite_id, int x, int y, bool h_flip, bool v_flip);
void rge_draw_scaled_sprite(BITMAP* layer, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int sprite_id);
void rge_draw_rotated_sprite(BITMAP* src, BITMAP* dest, int x, int y, bool clockwise);
void rge_draw_rect(BITMAP* layer, int x, int y, int width, int height);
void rge_draw_rect_no_fill(BITMAP* layer, int x, int y, int width, int height);
void rge_draw_circle(BITMAP* layer, int x, int y, int radius);
void rge_draw_circle_no_fill(BITMAP* layer, int x, int y, int radius);
void rge_draw_line(BITMAP* layer, int x1, int y1, int x2, int y2);
void rge_draw_triangle(BITMAP* layer, int x1, int y1, int x2, int y2, int x3, int y3);
void rge_draw_triangle_no_fill(BITMAP* layer, int x1, int y1, int x2, int y2, int x3, int y3);
void rge_draw_1bpp_sprite(BITMAP* layer, sprite_1bpp_t sprite, int x, int y); 
void rge_draw_2bpp_sprite(BITMAP* layer, sprite_2bpp_t sprite, int x, int y);
void rge_draw_4bpp_sprite(BITMAP* layer, sprite_4bpp_t sprite, int x, int y);

void rge_set_pixel(BITMAP* layer, int x, int y, int color_id);
int rge_get_pixel(BITMAP* layer, int x, int y);
#endif  //END __RGE_ENGINE_GRAPHICS_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// AUDIO SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_AUDIO_SYSTEM_BLOCK
typedef struct audio_sample {
	int id;
	rge_string_t* name;
	SAMPLE* sample;
	int volume;
	int pan;
	int pitch;
	bool loop;
} audio_sample_t;

typedef struct rge_audio_manager {
	arraystack_t audio_samples;
	iterator_t iterator;
	int global_volume;
	void (*set_global_volume)(int volume);
	void (*add_sample)(audio_sample_t sample);
	void (*play_sample)(char* name);
	void (*stop_sample)(char* name);
	void (*restart_sample)(char* name);
	void (*dispose_sample)(int id);

	void (*update)();
	void (*dispose)();
} audio_manager_t;
rge_state_t rge_audio_init();
void rge_audio_sample_init(audio_sample_t* sample, char* filename, char* name, int pan, int pitch, int volume, bool loop);
void rge_audio_manager_init();
void rge_audio_manager_add_sample(audio_sample_t sample);
void rge_audio_manager_dispose_sample(int id);
void rge_audio_manager_play_sample(char* name);
void rge_audio_manager_stop_sample(char* name);
void rge_audio_manager_dispose();
audio_sample_t* rge_audio_manager_get_sample_by_name(char* name);
void rge_audio_manager_set_global_volume(int volume);
void rge_audio_manager_update_indexes();

#endif   //END  __RGE_ENGINE_AUDIO_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// SCENE SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_SCENE_SYSTEM_BLOCK

typedef void (*rge_scene_void_func_t)();

//-----------------------------------------------------------------------------
// SCENE
//-----------------------------------------------------------------------------
//make sure to set valid callbacks in for update/draw/dispose functions and set init_complete
//before using scene_t
typedef struct scene {
	int id;
	rge_string_t* name;
	bool is_visible;
	bool is_active;
	bool init_complete;
	rge_settings_t settings;
	rge_scene_void_func_t draw;
	rge_scene_void_func_t update;
	rge_scene_void_func_t dispose;
} scene_t;
void rge_scene_init(scene_t* scene, char* name, rge_scene_void_func_t draw, rge_scene_void_func_t update, rge_scene_void_func_t dispose);

///////////////////////////////////////////////////////////////////////////////
// SCENE MANAGER
///////////////////////////////////////////////////////////////////////////////
typedef struct rge_scene_manager {
	arraystack_t scenes;
	iterator_t iterator;
	char* active_scene;
	void (*add_scene)(scene_t scene);
	void (*dispose_scene)(int id);

	void (*draw)();
	void (*update)();
	void (*dispose)();
} scene_manager_t;
void rge_scene_manager_add_scene(scene_t scene);
void rge_scene_manager_dispose_scene(int id);
void rge_scene_manager_update_indexes();
scene_t* rge_scene_manager_get_scene_by_name(char* name);
void rge_scene_manager_init();
void rge_scene_manager_draw();
void rge_scene_manager_update();
void rge_scene_manager_dispose();
#endif  //END __RGE_ENGINE_SCENE_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// UI SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_UI_SYSTEM_BLOCK

//-----------------------------------------------------------------------------
// UI - COMPONENT STATE
//-----------------------------------------------------------------------------
typedef enum ui_component_state {
	NORMAL,
	HIGHLIGHTED,
	ACTIVATED
} ui_component_state_t;

//-----------------------------------------------------------------------------
// UI - PADDING
//-----------------------------------------------------------------------------
typedef struct ui_padding {
	int left;
	int right;
	int top;
	int bottom;
} ui_padding_t;

//-----------------------------------------------------------------------------
// UI - COMPONENT PROPERTIES
//-----------------------------------------------------------------------------
typedef void (*rge_ui_callback_func_t)(char* callback_args);
typedef struct ui_component_props {
	int id;
	rge_string_t* scene_name;  //if null global ui component, otherwise scene name to render ui component with
	bool is_visible;
	rge_string_t* name;
	rge_string_t* msg;
	//text and highlight colors
	color_palette_index_t foreground;
	//button background colors
	color_palette_index_t background;
	//border highlight color
	color_palette_index_t border_highlight;
	ui_component_state_t state;
	rge_string_t* callback_args;
	rge_ui_callback_func_t activation_callback;
	ui_padding_t padding;
	int x;
	int y;
	int width;
	int height;
} ui_component_props_t;

//-----------------------------------------------------------------------------
// UI - COMPONENT
//-----------------------------------------------------------------------------
typedef struct ui_component {
	ui_component_props_t props;
	void (*draw)(ui_component_props_t* props);
	void (*update)(ui_component_props_t* props);
	void (*dispose)(ui_component_props_t* props);
} ui_component_t;
void rge_ui_component_init(ui_component_t* component, const char* name, const char* msg, const char* scene_name, ui_padding_t padding, int x, int y, int width, int height,
	color_palette_index_t foreground, color_palette_index_t background, color_palette_index_t border_highlight,
	void (*draw)(ui_component_props_t* props), void (*update)(ui_component_props_t* props), char* callback_args, rge_ui_callback_func_t activation_callback);
void rge_ui_component_init_auto_size(ui_component_t* component, const char* name, const char* scene_name, const char* msg, int x, int y,
	color_palette_index_t foreground, color_palette_index_t background, color_palette_index_t border_highlight,
	void (*draw)(ui_component_props_t* props), void (*update)(ui_component_props_t* props), char* callback_args, rge_ui_callback_func_t activation_callback);
rge_ui_callback_func_t rge_ui_component_no_callback();
void rge_ui_component_dispose(ui_component_props_t* props);

//-----------------------------------------------------------------------------
// UI - MULTILINE LABEL
//-----------------------------------------------------------------------------
typedef struct ui_multiline_label {
	ui_component_t component;
} ui_multiline_label_t;
void rge_ui_multiline_label_init(ui_multiline_label_t* label, const char* name, const char* msg, const char* scene_name, ui_padding_t padding, int x, int y, int width, int height,
	color_palette_index_t foreground, color_palette_index_t background, color_palette_index_t border_highlight);
void rge_ui_multiline_label_update(ui_component_props_t* props);
void rge_ui_multiline_label_draw(ui_component_props_t* props);

//-----------------------------------------------------------------------------
// UI - BUTTON
//-----------------------------------------------------------------------------
typedef struct ui_button {
	ui_component_t component;
} ui_button_t;
void rge_ui_button_init(ui_button_t* button, const char* name, const char* msg, const char* scene_name, int x, int y, int width, int height,
	color_palette_index_t foreground, color_palette_index_t background, color_palette_index_t border_highlight, char* callback_args, rge_ui_callback_func_t activation_callback);
void rge_ui_button_update(ui_component_props_t* props);
void rge_ui_button_draw(ui_component_props_t* props);

typedef struct button_input_state {
	bool pressed; //key is actively held down
	bool released; //held key was released

	//while key is pressed, pressed_ticks is updated
	//this is dependent on INPUT_MS or how frequently we poll input
	//so if we poll input every 50 MS, this means we will do a 
	//pressed duration tick 20 times per second  so to calculate
	//held duration we would do
	//pressed_ticks / (1000 / INPUT_MS) for seconds held (fractional seconds if we use a float to compute this)
	u8 pressed_ticks;
	u8 max_tick_count;  //how many ticks before we stop incrementing (essentially how many seconds)
	u8 tick_growth_rate;  //how many ticks to add per input poll (pressed)
	u8 tick_decay_rate; //how many tick to remove per input poll (not pressed)
} button_input_state_t;

///////////////////////////////////////////////////////////////////////////////
// UI MANAGER
///////////////////////////////////////////////////////////////////////////////
typedef struct rge_ui_manager {
	arraystack_t components;
	iterator_t iterator;
	void (*add_component)(ui_component_t component);
	void (*dispose_component)(int id);

	void (*draw)();
	void (*update)();
	void (*dispose)();
} ui_manager_t;
void rge_ui_manager_add_component(ui_component_t component);
void rge_ui_manager_dispose_component(int id);
void rge_ui_manager_update_indexes();
ui_component_t* rge_ui_manager_get_component_by_name(char* name);
void rge_ui_manager_init();
void rge_ui_manager_update();
void rge_ui_manager_draw();
void rge_ui_manager_dispose();
#endif  //END __RGE_ENGINE_UI_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// INPUT SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_INPUT_SYSTEM_BLOCK

typedef enum input_mode {
	SIMPLE,  //pressed or released states only
	PRESS_DURATION_ONLY, //allow duration held ticks
	PRESS_DURATION_AND_DECAY //allow duration held ticks and decay ticks
} input_mode_t;

typedef struct input_state {
#ifdef RGE_DEBUG
	button_input_state_t debug_toggle_key_state;
	button_input_state_t debug_next_msg_key_state;
	button_input_state_t debug_set_mode_key_state;
#endif

	input_mode_t mode;

	//dpad
	button_input_state_t up;
	button_input_state_t down;
	button_input_state_t left;
	button_input_state_t right;

	//buttons
	button_input_state_t a;
	button_input_state_t b;
	button_input_state_t start;
	button_input_state_t select;

	button_input_state_t shutdown_key;

	//extended keyboard functions for editors
	//we only support bindings we are using in the editors
#ifdef RGE_EXTENDED_INPUT
	button_input_state_t kb_esc;
	button_input_state_t kb_comma;
	button_input_state_t kb_enter;
	button_input_state_t kb_period;
	button_input_state_t kb_single_qoute;
	button_input_state_t kb_semicolon;
	button_input_state_t kb_open_sqr_bracket;
	button_input_state_t kb_close_sqr_bracket;
	button_input_state_t kb_slash;
	button_input_state_t kb_backslash;
	button_input_state_t kb_backspace;
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

	button_input_state_t kb_up;
	button_input_state_t kb_down;
	button_input_state_t kb_left;
	button_input_state_t kb_right;

	button_input_state_t kb_pgup;
	button_input_state_t kb_pgdn;

	button_input_state_t kb_del;
	button_input_state_t kb_tab;
	button_input_state_t kb_space;

	button_input_state_t kb_a;
	button_input_state_t kb_b;
	button_input_state_t kb_c;
	button_input_state_t kb_d;
	button_input_state_t kb_e;
	button_input_state_t kb_f;
	button_input_state_t kb_g;
	button_input_state_t kb_h;
	button_input_state_t kb_i;
	button_input_state_t kb_j;
	button_input_state_t kb_k;
	button_input_state_t kb_l;
	button_input_state_t kb_m;
	button_input_state_t kb_n;
	button_input_state_t kb_o;
	button_input_state_t kb_p;
	button_input_state_t kb_q;
	button_input_state_t kb_r;
	button_input_state_t kb_s;
	button_input_state_t kb_t;
	button_input_state_t kb_u;
	button_input_state_t kb_v;
	button_input_state_t kb_w;
	button_input_state_t kb_x;
	button_input_state_t kb_y;
	button_input_state_t kb_z;
	button_input_state_t kb_0;
	button_input_state_t kb_1;
	button_input_state_t kb_2;
	button_input_state_t kb_3;
	button_input_state_t kb_4;
	button_input_state_t kb_5;
	button_input_state_t kb_6;
	button_input_state_t kb_7;
	button_input_state_t kb_8;
	button_input_state_t kb_9;
#endif
} input_state_t;

void rge_enable_button_pressed_duration(button_input_state_t* button, u8 max_seconds, u8 tick_decay_rate, u8 tick_growth_rate);
bool rge_button_was_released(button_input_state_t* button);
void rge_input();

#endif   //END  __RGE_ENGINE_INPUT_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// FILESYSTEM, IMPORT/EXPORT FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_FILE_SYSTEM_BLOCK
typedef enum sag_filetype {
	SAG,
	BMP,
	C_HEADER
} sag_filetype_t;

typedef enum sag_file_status {
	FILE_EXPORTED,
	FILE_EXISTS_NOT_EXPORTED,
	FILE_ERROR_UNKNOWN_NOT_EXPORTED,
	FILE_IMPORTED,
	FILE_ERROR_UNKNOWN_NOT_IMPORTED
} sag_file_status_t;

//imports audio, graphics, maps from file, if BMP then only sprites are imported
sag_file_status_t import_sag_file(sprite_palette_format_t format, const char* filename, sag_filetype_t filetype);
//export audio, graphics, maps etc... to file, if the filetype is BMP only
//the sprites are exported
sag_file_status_t export_sag_file(sprite_palette_format_t format, const char* filename, sag_filetype_t filetype);
#endif  //END __RGE_ENGINE_FILE_SYSTEM_BLOCK


///////////////////////////////////////////////////////////////////////////////
// DEBUG SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_DEBUG_SYSTEM_BLOCK

#ifdef RGE_DEBUG
	typedef enum debug_mode {
		DEBUG_PALETTE,
		DEBUG_VRAM,
		DEBUG_NO_OVERLAY
	} debug_mode_t;

	typedef struct debug {
		bool visible;
		debug_mode_t debug_mode;
		//I don't want to do heap allocation and space isn't an issue for memory
		//on modern Windows so just aloc 5 messages @ 256 chars
		//last message in list is always reserved for
		//frame_rate and main ops metric
		u8 active_message_index;
		char message[DEBUG_MSG_COUNT][DEBUG_MSG_SIZE];
		u8 frame_rate;
		u16 frame_ticks_sec;
		u8 main_ops;
		u16 main_ticks_sec;
	} debug_t;

	void rge_debug_reset(); //reset frame_ticks and main ops for the last second (for fps and main ops feedback)
	void rge_debug_text(u16 x, u16 y, const char* message, text_align_t text_alignment);
	void rge_debug_palette();  //prints palette to screen with index for each color
	void rge_debug_vram();  //prints vram to screen with index for each tile
	void rge_set_debug_msg(u8 debug_msg_index, const char* msg);
#endif

#endif  //END __RGE_ENGINE_DEBUG_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// RGE GLOBALS
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_GLOBALS_BLOCK
extern BITMAP* ui_layer;
extern BITMAP* sprite_layer;
extern BITMAP* background_layer;
extern BITMAP* back_buffer;
extern BITMAP* vram;
extern BITMAP* sprites[256];
extern u8 flags_for_sprites[256];

#ifdef RGE_DEBUG
	extern volatile debug_t rge_debug;  //defined in rge.c
#endif
extern volatile bool rge_shutdown; 
extern volatile bool frame_tick; 
extern volatile bool audio_tick; 
extern volatile bool input_tick; 
extern volatile unsigned int global_timer_tick;

extern graphics_t rge_gfx; 
extern input_state_t rge_input_state; 
extern scene_manager_t rge_scene_manager;
extern ui_manager_t rge_ui_manager; 
extern audio_manager_t rge_audio_manager;
#endif  //END __RGE_ENGINE_GLOBALS_BLOCK

///////////////////////////////////////////////////////////////////////////////
// RGE TIMER SYSTEM
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_TIMER_SYSTEM_BLOCK

//to keep timing system we track globally the number of milliseconds that have passed since program 
//start, look under GLOBALS section for extern global_timer_tick definition
//NOTE:  int is used which will be platform specific we assume @ least a 32 bit integer before we overflow
//this means @ 1000 ms * 60 seconds * 60 minutes * 24 hours = 86,400,000 ms per day and at 2^32 before overflow we
//can run the engine for ~49 days without issue, what is an issue is when the platform lacks 32 bit integer support
void rge_global_timer_tick();

int rge_millis_since_start();
int rge_seconds_since_start();
int rge_minutes_since_start();

#endif  //END __RGE_TIMER_SYSTEM_BLOCK

///////////////////////////////////////////////////////////////////////////////
// ENGINE LIFECYCLE
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_LIFECYCLE_BLOCK

void rge_redraw();
//void rge_buffer_audio();
void rge_poll_input();

rge_state_t rge_init(screen_resolution_t resolution, const char* sag_path);
void rge_update();
void rge_dispose();
#endif  //END __RGE_ENGINE_LIFECYCLE_BLOCK 

#endif

///////////////////////////////////////////////////////////////////////////////
// FEATURES/BUGS/ISSUES
///////////////////////////////////////////////////////////////////////////////
#ifndef __RGE_ENGINE_BUGS_AND_ISSUES_BLOCK

//ISSUES:
/**
----------------------------------------------
ISSUE: input when switching scenes is captured
This means that if I hit WASD, etc... on scene that isn't automatically clearing its state back from pressed then
when we return to a scene that does track this the keys will fire
----------------------------------------------
*/

//BREAKING CHANGES/FEATURE REMOVAL
/**

----------------------------------------------
CHANGE:  I've never liked the UI system and it feels unneccesary given how easy it is with the current input
and graphics API to make UI, when safe to do so look at removing
----------------------------------------------
*/

//FEATURES TO IMPLEMENT:
/**

----------------------------------------------
FEATURE:  scene management is very cumbersome at the moment, it would be nice to overhaul and
add additional methods for resume/pause when moving to another scene.  Furthermore it should
make use of a simple API call like the graphics system.  Something like ADD_SCENE(), REMOVE_SCENE(),
GET_SCENE(), etc...
----------------------------------------------

----------------------------------------------
FEATURE:  settings management is very cumbersome (similar to scene management) convert methods to simple
MACROs and implement a simple API call like ADD_SETTING(), REMOVE_SETTING(), GET_SETTING
----------------------------------------------
*/
#endif  //END __RGE_ENGINE_BUGS_AND_ISSUES_BLOCK
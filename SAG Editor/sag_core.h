#ifndef __2D_RGE_SAG_EDITOR_CORE
#define __2D_RGE_SAG_EDITOR_CORE

#include "../RetroGE/RetroGE/rge.h"

//where we export and import from
#define SAG_ASSETS_PATH "./Assets/"

#define SAG_WHITE 255
#define SAG_BLACK 254
#define SAG_GREY 28
#define SAG_LIGHT_GREY 30
#define SAG_PINK 13
#define SAG_LIGHT_PINK 62
#define SAG_BROWN 6
#define SAG_GREEN 10
#define SAG_PURPLE 9
#define SAG_BLUE 54
#define SAG_ORANGE 43

//one more then actual requirement (so if we want 8 char filename, use 9 here)
//we use one of the chars to store the cursor/next char
#define SAG_MAX_FILENAME_LEN 9 //for now does 8.3 filename support only
#define SAG_MAX_FILE_EXT_LEN 4 //.XXX

//'File exists not saved.\0'  //max len 23, 64 for some leeway
#define SAG_MAX_STATUS_MSG_LEN 64

typedef enum sag_movement {
	SAG_EDITOR_UP,
	SAG_EDITOR_DOWN,
	SAG_EDITOR_LEFT,
	SAG_EDITOR_RIGHT
} sag_movement_t;

//draw a cursor at position x,y
void sag_draw_cursor(int x, int y);
//draw the top ui bar and bottom ui bar
void sag_draw_ui_bars();
//draw sag editor global key bindings (F1,F2, etc...)
void sag_draw_global_keys();
//clear the screen
void sag_clear_screen();

//add a char to the keyb_input buffer, we use keybd_input buffer to capture
//things like filenames or other user entered character data
void sag_set_keybd_input_char(char* keybd_input, int* keybd_index, char value, int max_length);
//check for editor bindings, ie: a-z,0..9,enter,esc, etc...
void sag_get_keybd_input(char* keybd_input, int* keybd_index, int max_length);
//reset the state of all keys we are watching in the editor (as the engine currently does not do this)
void sag_reset_keybd(char* keybd_input, int* keybd_index, int max_length);

#endif


///////////////////////////////////////////////////////////////////////////////
// FEATURES/BUGS/ISSUES
///////////////////////////////////////////////////////////////////////////////
#ifndef __2D_RGE_SAG_EDITOR_BUGS_AND_ISSUES_BLOCK

//ISSUES:
/**
----------------------------------------------
ISSUE: issue name
Issue summary goes here....
----------------------------------------------
*/

//BREAKING CHANGES/FEATURE REMOVAL
/**

----------------------------------------------
CHANGE:  change name
Change summary goes here....
----------------------------------------------
*/

//FEATURES TO IMPLEMENT:
/**

----------------------------------------------
FEATURE:  Export directly back to BMP
Export directly to a BMP from the export window.
----------------------------------------------
*/
#endif  //END __2D_RGE_SAG_EDITOR_BUGS_AND_ISSUES_BLOCK
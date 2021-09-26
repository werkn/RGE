#ifndef __2D_RGE_PALETTES
#define __2D_RGE_PALETTES

#include <allegro.h>

#define RGE_CLEAR_COLOR_INDEX 3
#define RGE_DEFAULT_FOREGROUND_INDEX 1
#define RGE_DEBUG_TEXT_BACKGROUND 254
#define RGE_DEBUG_TEXT_FOREGROUND 255

//NOTE: Allegro 4.4 oddity here
//PALETTES CANNOT BE CONST under Allegro, I'm assuming because the palette itself has 
//@ least index 0 changed to represent clear color
extern PALETTE RGE_PALETTES; //defined in rge_palettes.c

#endif
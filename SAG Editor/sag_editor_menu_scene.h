#ifndef __2D_RGE_EDITOR_MENU_SCENE
#define __2D_RGE_EDITOR_MENU_SCENE

#include "./RetroGE/rge.h"
#include "./sag_core.h"

typedef enum sag_editor_menu_mode {
	IMPORT_AND_EXPORT_OPTIONS,
	IMPORT_FILE,
	EXPORT_FILE
} sag_editor_menu_mode_t;

scene_t scene_sag_editor_menu_init(char* active_background_scene);

#endif
#include "./sag_editor_menu_scene.h"

sag_editor_menu_mode_t sag_menu_mode; //what menu option are we in
sag_filetype_t sag_active_filetype; //what type of file were importing or exporting
sprite_palette_format_t sag_active_sprite_format;
char sag_filename[SAG_MAX_FILENAME_LEN + SAG_MAX_FILE_EXT_LEN];
char sag_status_bar_text[SAG_MAX_STATUS_MSG_LEN];
bool sag_keybd_reset = false;

char keybd_input[SAG_MAX_FILENAME_LEN];
int keybd_index = 0;

void sag_draw_editor_menu_options() {
    //draw menu label
    FILL(SAG_GREY);
    RECTFILL(ui_layer, 25, 24, 135, 20);
    TEXT(ui_layer, 26, 32, "IMPORT/EXPORT", SAG_WHITE, SAG_GREY, TEXT_ALIGN_LEFT);

    TEXT(ui_layer, 27, 48, "[0] Imp (.sag)", SAG_WHITE, SAG_GREEN, TEXT_ALIGN_LEFT);
    TEXT(ui_layer, 27, 58, "[1] Imp (.bmp)", SAG_WHITE, SAG_ORANGE, TEXT_ALIGN_LEFT);

    STROKE(SAG_WHITE);
    LINE(ui_layer, 160, 48, 160, 136);

    TEXT(ui_layer, 293, 48, "[2] Exp (.sag)", SAG_WHITE, SAG_BLUE, TEXT_ALIGN_RIGHT);
    TEXT(ui_layer, 293, 58, "[3] Exp (.bmp)", SAG_WHITE, SAG_PURPLE, TEXT_ALIGN_RIGHT);
    TEXT(ui_layer, 293, 68, "[4] Exp   (.h)", SAG_WHITE, SAG_PINK, TEXT_ALIGN_RIGHT);

    TEXT(ui_layer, 200, 210, "[ESC] Go back", SAG_WHITE, SAG_PINK, TEXT_ALIGN_LEFT);
}

void sag_draw_editor_file_dlg() {
    
    //draw menu label
    FILL(SAG_GREY);
    RECTFILL(ui_layer, 25, 24, 135, 20);

    switch (sag_menu_mode) {
    case IMPORT_FILE:
        TEXT(ui_layer, 26, 32, "IMPORT FILE", SAG_WHITE, SAG_GREY, TEXT_ALIGN_LEFT);
        break;
    case EXPORT_FILE:
        TEXT(ui_layer, 26, 32, "EXPORT FILE", SAG_WHITE, SAG_GREY, TEXT_ALIGN_LEFT);
        break;
    }

    TEXT(ui_layer, 40, 56, "FILENAME:", SAG_WHITE, SAG_LIGHT_PINK, TEXT_ALIGN_LEFT);
    FILL(SAG_LIGHT_GREY);
    RECTFILL(ui_layer, 38, 68, 240, 20);
    STROKE(SAG_BLACK);
    RECT(ui_layer, 38, 68, 240, 20);

    TEXT(ui_layer, 42, 76, sag_filename, SAG_BLACK, SAG_LIGHT_GREY, TEXT_ALIGN_LEFT);

    TEXT(ui_layer, 180, 100, "[ENTER] OK", SAG_WHITE, SAG_LIGHT_PINK, TEXT_ALIGN_LEFT);

    TEXT(ui_layer, 200, 210, "[ESC] Go back", SAG_WHITE, SAG_PINK, TEXT_ALIGN_LEFT);
}

void scene_sag_editor_menu_exit() {
    //destroy this scene and exit to last scene
    scene_t* tmp_scene = rge_scene_manager_get_scene_by_name("sag_editor_menu");

    char* background_scene = tmp_scene->settings.get_string_setting(&tmp_scene->settings, "background_scene");
    rge_scene_manager_get_scene_by_name(background_scene)->is_active = true;

    //destroy this scene and transition 
    rge_scene_manager.dispose_scene(tmp_scene->id);
}

void scene_sag_editor_menu_update() {

    switch (sag_menu_mode) {
    case IMPORT_AND_EXPORT_OPTIONS:    
        //handle menu selections
        //import .SAG
        if (rge_input_state.kb_0.released) {
            rge_input_state.kb_0.released = false;
            sag_menu_mode = IMPORT_FILE;
            sag_active_filetype = SAG;
        }
        else if (rge_input_state.kb_1.released) {
            rge_input_state.kb_1.released = false;
            sag_menu_mode = IMPORT_FILE;
            sag_active_filetype = BMP;
        }
        else if (rge_input_state.kb_2.released) {
            rge_input_state.kb_2.released = false;
            sag_menu_mode = EXPORT_FILE;
            sag_active_filetype = SAG;
        }
        else if (rge_input_state.kb_3.released) {
            rge_input_state.kb_3.released = false;
            sag_menu_mode = EXPORT_FILE;
            sag_active_filetype = BMP;
        }
        else if (rge_input_state.kb_4.released) {
            rge_input_state.kb_4.released = false;
            sag_menu_mode = EXPORT_FILE;
            sag_active_filetype = C_HEADER;
        } 
        else if (rge_input_state.kb_esc.released) {
            rge_input_state.kb_esc.released = false;
            sag_menu_mode = IMPORT_AND_EXPORT_OPTIONS;
            scene_sag_editor_menu_exit();
        }
        
        break;
    case IMPORT_FILE:
    case EXPORT_FILE:
        
        if (!sag_keybd_reset) {
            sag_keybd_reset = true;
            sag_reset_keybd(keybd_input, &keybd_index, SAG_MAX_FILENAME_LEN);
        }

        //when export DLG is shown a-z is captured for filename
        sag_get_keybd_input(keybd_input, &keybd_index, SAG_MAX_FILENAME_LEN);

        //update filename
        switch (sag_active_filetype) {
        case SAG:
            snprintf(sag_filename, SAG_MAX_FILENAME_LEN + SAG_MAX_FILE_EXT_LEN, "%s.sag", keybd_input);
            break;
        case BMP:
            snprintf(sag_filename, SAG_MAX_FILENAME_LEN + SAG_MAX_FILE_EXT_LEN, "%s.bmp", keybd_input);
            break;
        case C_HEADER:
            snprintf(sag_filename, SAG_MAX_FILENAME_LEN + SAG_MAX_FILE_EXT_LEN, "%s.h", keybd_input);
            break;
        }

        //check for sumbit
        //note: enter is the default start key so .start.released works as well here
        if (rge_input_state.kb_enter.released) {
            rge_input_state.kb_enter.released = false;

            char full_path[64];
            snprintf(full_path, 64, "%s%s", SAG_ASSETS_PATH, sag_filename);

            //import or export
            sag_file_status_t result;
            switch (sag_menu_mode) {
            case EXPORT_FILE:
                //attempt to export file, update status bar with message (ie: saved, didn't save, duplicate file, etc...)
                result = export_sag_file(sag_active_sprite_format, full_path, sag_active_filetype);

                switch (result) {
                case FILE_EXISTS_NOT_EXPORTED:
                    snprintf(sag_status_bar_text, SAG_MAX_STATUS_MSG_LEN, "File '%s' exists!", sag_filename);
                    break;
                case FILE_EXPORTED:
                    snprintf(sag_status_bar_text, SAG_MAX_STATUS_MSG_LEN, "File '%s' exported.", sag_filename);
                    break;
                default:
                case FILE_ERROR_UNKNOWN_NOT_EXPORTED:
                    snprintf(sag_status_bar_text, SAG_MAX_STATUS_MSG_LEN, "File not exported, unknown error!");
                    break;
                }
                break;
            case IMPORT_FILE:
                //attempt to export file, update status bar with message (ie: saved, didn't save, duplicate file, etc...)
                result = import_sag_file(sag_active_sprite_format, full_path, sag_active_filetype);

                switch (result) {
                case FILE_IMPORTED:
                    snprintf(sag_status_bar_text, SAG_MAX_STATUS_MSG_LEN, "File '%s' imported!", sag_filename);
                    break;
                default:
                case FILE_ERROR_UNKNOWN_NOT_IMPORTED:
                    snprintf(sag_status_bar_text, SAG_MAX_STATUS_MSG_LEN, "File not imported, unknown error!");
                    break;
                }
                break;
            }

            sag_menu_mode = IMPORT_AND_EXPORT_OPTIONS;

            //mark keyboard as needing reset
            sag_keybd_reset = false;
        } else if (rge_input_state.kb_esc.released) {
                rge_input_state.kb_esc.released = false;
                sag_menu_mode = IMPORT_AND_EXPORT_OPTIONS;
                //mark keyboard as needing reset
                sag_keybd_reset = false;
        }
        break;
    }
}

void scene_sag_editor_menu_draw() {
    sag_clear_screen();
    sag_draw_ui_bars();
    //draw background contaner
    FILL(SAG_LIGHT_PINK);
    RECTFILL(ui_layer, 25, 44, 270, 100);

    switch (sag_menu_mode) {
    case IMPORT_AND_EXPORT_OPTIONS:
        sag_draw_editor_menu_options();
        break;
    case IMPORT_FILE:
    case EXPORT_FILE:
        sag_draw_editor_file_dlg();
        break;
    }

    //draw status text
    TEXT(ui_layer, 10, 232, sag_status_bar_text, SAG_WHITE, SAG_PINK, TEXT_ALIGN_LEFT);
}

void scene_sag_editor_menu_dispose() {

}

scene_t scene_sag_editor_menu_init(char* active_background_scene) {

    //create a scene
    scene_t scene_sag_editor_menu;
    rge_scene_init(&scene_sag_editor_menu, "sag_editor_menu",
        scene_sag_editor_menu_draw,
        scene_sag_editor_menu_update,
        scene_sag_editor_menu_dispose);

    //save active_background_scene for later (we restore it when dismissing menu)
    scene_sag_editor_menu.settings.add_setting(&scene_sag_editor_menu.settings, RGE_STRING, "background_scene", active_background_scene);

    sag_reset_keybd(keybd_input, &keybd_index, SAG_MAX_FILENAME_LEN);

    sag_menu_mode = IMPORT_AND_EXPORT_OPTIONS;
    sag_active_sprite_format = FORMAT_1BPP;
    sag_active_filetype = SAG;

    return scene_sag_editor_menu;
}
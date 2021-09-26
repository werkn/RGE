#include "../Sprite Editor/sprite_editor_scene.h"
#include "../sag_editor_menu_scene.h"

//drawing canvas
sag_sprite_editor_tool_t active_tool;
char tool_text[2][5] = {
    "PEN",
    "FILL"
};

int cursor_row = 0;
int cursor_column = 0;

//palette color
int palette_column = 12;

//anim preview settings
int anim_start = 0; //start frame id
int anim_end = 0;  //last frame id
int anim_speed = 1;  //frames per second
int anim_frame = 0;
bool playing_anim = false;
int last_millis = 0;

int sag_active_anim_frame() {
    return (anim_start + anim_frame) % 256;
}

int sag_reset_anim() {
    anim_frame = 0;
    last_millis = 0;
    playing_anim = false;
}

int sag_next_anim_preview_sprite() {
    int current_millis = MILLIS();
    
    //render the current frame based on anim FPS
    if (current_millis - last_millis > (1000 / anim_speed)) {
        last_millis = MILLIS();
        anim_frame = ((anim_frame + 1) <= (anim_end - anim_start)) ? anim_frame + 1 : 0;
    }
}

//sprite page
int sprite_page = 0;
int sprite_column = 0;
int sprite_row = 0;

//we should probably convert this to an int and bitmask but meh
//we'll replace it with our SAG file exporter at some point
bool sprite_flags[256][8];

int sag_active_sprite_id() {
    return (sprite_page * 16) + (sprite_row * 8 + (sprite_column));
}

void sag_draw_sprite_editor_canvas() {
    //draw sprite id
    //"ID: 256\0" max length of 8
    char active_sprite[8];
    snprintf(active_sprite, 8, "ID: %d", sag_active_sprite_id());
    TEXT(ui_layer, SAG_SPRITE_ID_X, SAG_SPRITE_ID_Y, active_sprite, SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);
    STROKE(SAG_WHITE);
 
    //draw the currenlty selected sprite
    SSPR(sprite_layer, 0, 0, 16, 16, 
        SAG_EDITOR_CANVAS_X, SAG_EDITOR_CANVAS_Y, 
        SAG_EDITOR_CANVAS_W, SAG_EDITOR_CANVAS_H, 
        sag_active_sprite_id());
    RECT(ui_layer, SAG_EDITOR_CANVAS_X, SAG_EDITOR_CANVAS_Y, SAG_EDITOR_CANVAS_W, SAG_EDITOR_CANVAS_H);

    //draw cursor showing current tile being drawn on
    STROKE(SAG_WHITE);
    sag_draw_cursor(SAG_EDITOR_CANVAS_X + (cursor_column * SAG_EDITOR_CANVAS_W/8), 
        SAG_EDITOR_CANVAS_Y + (cursor_row * SAG_EDITOR_CANVAS_H/8));
}

void sag_draw_sprite_flags() {
    TEXT(ui_layer, 135, 30, "FLAGS", SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);
    STROKE(SAG_WHITE);
    RECT(ui_layer, 135, 40, 164, 20);

    //draw each of the individual flag bits
    for (int flag_bit = 0; flag_bit < 8; flag_bit++) {
        //fill if set
        if (sprite_flags[sag_active_sprite_id()][flag_bit]) {
            FILL(flag_bit+1); //+1 to exclude clear color
            RECTFILL(ui_layer, 142 + (flag_bit * 20), 44, 10, 10);
        }
        STROKE(SAG_WHITE);
        RECT(ui_layer, 142 + (flag_bit * 20), 44, 10, 10);
    }
}

void sag_draw_sprite_preview() {
    TEXT(ui_layer, 135, 70, "PREVIEW", SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);
    STROKE(SAG_WHITE);
    RECT(ui_layer, 135, 80, 60, 60);
    
    //draw each of the individual flag bits
    for (int column = 0; column < 3; column++) {
        for (int row = 0; row < 3; row++) {
            //dummy sprite 
            SSPR(sprite_layer, 0, 0, 16, 16, 
                135 + (column*20), 
                80 + (row*20), 
                20, 20, sag_active_sprite_id());
        }
    }
}

void sag_draw_current_palette() {

    //!!!!!!!!!!!
    //TODO 16<---- needs to be converted to the size of the palette being used!
    for (int color = 0; color < 16; color++) {
        FILL(color+1);  //0 is transparent atm
        RECTFILL(sprite_layer, SAG_EDITOR_PALETTE_X + (color * 16), SAG_EDITOR_PALETTE_Y, 16, 16);
    }

    STROKE(SAG_WHITE);
    RECT(ui_layer, SAG_EDITOR_PALETTE_X, SAG_EDITOR_PALETTE_Y, SAG_EDITOR_PALETTE_W, SAG_EDITOR_PALETTE_H);

    sag_draw_cursor(SAG_EDITOR_PALETTE_X + (palette_column * SAG_EDITOR_PALETTE_W / 16),
        SAG_EDITOR_PALETTE_Y);
    STROKE(SAG_WHITE);
    RECT(ui_layer, SAG_EDITOR_PALETTE_X +(palette_column * SAG_EDITOR_PALETTE_W / 16),
        SAG_EDITOR_PALETTE_Y, 16, 16);
}

void sag_draw_animation_preview() {
    TEXT(ui_layer, 200, 70, "ANIM PREVIEW", SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);
    
    //dummy sprite
    SSPR(sprite_layer, 0, 0, 16, 16, 200, 80, 60, 60, sag_active_anim_frame());
    STROKE(SAG_WHITE);
    RECT(ui_layer, 200, 80, 60, 60);

    //start sprite id, end sprite id and framerate to preview at
    //'Sp: 255\0', max 8
    char anim_text[8];
    snprintf(anim_text, 8, "S: %d", anim_start);
    TEXT(ui_layer, 264, 80, anim_text, SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);
    snprintf(anim_text, 8, "Sp: %d", anim_speed);
    TEXT(ui_layer, 264, 90, anim_text, SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);
    snprintf(anim_text, 8, "E: %d", anim_end);
    TEXT(ui_layer, 264, 100, anim_text, SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);
}

void sag_draw_sprite_page(int page) {
    STROKE(SAG_WHITE);
    RECT(ui_layer, SAG_EDITOR_SPRITE_PAGE_X, SAG_EDITOR_SPRITE_PAGE_Y,
        SAG_EDITOR_SPRITE_PAGE_W, SAG_EDITOR_SPRITE_PAGE_H);

    //"Page 16/16\0" max length 11
    char page_number[11];
    snprintf(page_number, 11, "Page %d/16", page+1);
    TEXT(ui_layer, SAG_EDITOR_SPRITE_PAGE_X + SAG_EDITOR_SPRITE_PAGE_W + 5, 
        SAG_EDITOR_SPRITE_PAGE_Y, page_number, SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);

    int start_sprite_id = (page) * 16;
    int row = 0;
    for (int sprite_id = start_sprite_id; sprite_id < start_sprite_id + 16; sprite_id++) {
        //dummy sprite 
        SSPR(sprite_layer, 0, 0, 16, 16,
            SAG_EDITOR_SPRITE_PAGE_X + ((sprite_id - (page)*16) % 8) * 24,
            SAG_EDITOR_SPRITE_PAGE_Y + ((sprite_id - (page)*16) / 8) * 24,
            24, 24, sprite_id);
    }

    //highlight active sprite (this is the one in canvas for editing)
    RECT(ui_layer, 
        SAG_EDITOR_SPRITE_PAGE_X + (sprite_column * SAG_EDITOR_SPRITE_PAGE_W / 8),
        SAG_EDITOR_SPRITE_PAGE_Y + (sprite_row * 24),
        24, 24);
}

void sag_flip_sprite(int sprite_id, bool h_flip, bool v_flip) {
    FILL(1);  //fill with clear color
    RECTFILL(sprites[255], 0, 0, 16, 16);
    //draw in sprite were flipping, and flip
    SPR(sprites[255], 0, 0, sprite_id, h_flip, v_flip);

    FILL(1);  //fill with clear color
    RECTFILL(sprites[sprite_id], 0, 0, 16, 16);
    //copy updated sprite from tmp back 
    SPR(sprites[sprite_id], 0, 0, 255, false, false);
}

void sag_rotate_sprite(int sprite_id) {
    FILL(1);  //fill with clear color
    RECTFILL(sprites[255], 0, 0, 16, 16);
    //draw in sprite were rotating
    SPRROT(sprites[sprite_id], sprites[255], 0, 0, true);

    FILL(1);  //fill with clear color
    RECTFILL(sprites[sprite_id], 0, 0, 16, 16);
    //copy updated sprite from tmp back 
    SPR(sprites[sprite_id], 0, 0, 255, false, false);
}

//moving active sprite will cause wrapping (toroidal structure)
void sag_move_sprite(int sprite_id, sag_movement_t direction) {
    FILL(1);  //fill with clear color
    RECTFILL(sprites[255], 0, 0, 16, 16);

    switch (direction) {
    case SAG_EDITOR_UP:
        SPR(sprites[255], 0, -2, sprite_id, false, false);
        SPR(sprites[255], 0, 14, sprite_id, false, false);
        break;
    case SAG_EDITOR_DOWN:
        SPR(sprites[255], 0, 2, sprite_id, false, false);
        SPR(sprites[255], 0, -14, sprite_id, false, false);
        break;
    case SAG_EDITOR_LEFT:
        SPR(sprites[255], -2, 0, sprite_id, false, false);
        SPR(sprites[255], 14, 0, sprite_id, false, false);
        break;
    case SAG_EDITOR_RIGHT:
        SPR(sprites[255], 2, 0, sprite_id, false, false);
        SPR(sprites[255], -14, 0, sprite_id, false, false);
        break;
    }

    FILL(1);  //fill with clear color
    RECTFILL(sprites[sprite_id], 0, 0, 16, 16);
    //copy updated sprite from tmp back 
    SPR(sprites[sprite_id], 0, 0, 255, false, false);
}

//recursive flood fill implementation, may exhaust stack!
//worst case is ~256 (16x16 pixels) recursive call
//should update to do 2x2 pixel which would mean ~64 recusive calls worst case
void sag_flood_fill(int replace_color_id, int fill_color_id, int x, int y, int sprite_id) {

    //don't waste any time processing fill on duplicate color
    if (replace_color_id == fill_color_id) {
        return;
    }

    if ((0 <= y && y < 16) && (0 <= x && x < 16)  //check were within texture bounds
        //verify that the current pixel is the color were filling
        && (PGET(sprites[sprite_id], x, y) == replace_color_id)) {
        //set color for current pixel
        PSET(sprites[sprite_id], x, y, fill_color_id);
        //check south pixel, call recursive
        sag_flood_fill(replace_color_id, fill_color_id, x, y - 1, sprite_id);
        //check north
        sag_flood_fill(replace_color_id, fill_color_id, x, y + 1, sprite_id);
        //check west
        sag_flood_fill(replace_color_id, fill_color_id, x - 1, y, sprite_id);
        //check east
        sag_flood_fill(replace_color_id, fill_color_id, x + 1, y, sprite_id);
    }

    return;
}

void sag_select_next_tool() {
    if ((active_tool + 1) != END_OF_TOOLS) {
        active_tool++;
    } else {
        active_tool = 0;  //pencil
    }
}

void scene_sprite_editor_exit_game(char* args) {
    ui_component_t* exit_game_btn = rge_ui_manager_get_component_by_name("exit_game");
    exit_game_btn->props.state = HIGHLIGHTED;

    //destroy this scene and exit game
    scene_t* tmp = rge_scene_manager_get_scene_by_name("sprite_editor");
    rge_scene_manager.dispose_scene(tmp->id);
    rge_shutdown = true;
}

void sag_canvas_input() {
    //move canvas cursor
    if (rge_input_state.left.released) {
        cursor_column = (cursor_column > 0) ? cursor_column - 1 : 7;
        //limit cursor movement to 8 pixels across 
        rge_input_state.left.released = false;
    }
    else if (rge_input_state.right.released) {
        cursor_column = (cursor_column < 7) ? cursor_column + 1 : 0;
        rge_input_state.right.released = false;
    }
    else if (rge_input_state.up.released) {
        cursor_row = (cursor_row > 0) ? cursor_row - 1 : 7;
        rge_input_state.up.released = false;
    }
    else if (rge_input_state.down.released) {
        cursor_row = (cursor_row < 7) ? cursor_row + 1 : 0;
        rge_input_state.down.released = false;
    }
}
void sag_move_canvas_input() {

    //sprite translation UP, DOWN, LEFT, RIGHT by 1PX using arrows
    if (rge_input_state.kb_up.released) {
        sag_move_sprite(sag_active_sprite_id(), SAG_EDITOR_UP);
        rge_input_state.kb_up.released = false;
    }
    else if (rge_input_state.kb_down.released) {
        sag_move_sprite(sag_active_sprite_id(), SAG_EDITOR_DOWN);
        rge_input_state.kb_down.released = false;
    }
    else if (rge_input_state.kb_left.released) {
        sag_move_sprite(sag_active_sprite_id(), SAG_EDITOR_LEFT);
        rge_input_state.kb_left.released = false;
    }
    else if (rge_input_state.kb_right.released) {
        sag_move_sprite(sag_active_sprite_id(), SAG_EDITOR_RIGHT);
        rge_input_state.kb_right.released = false;
    }

}
void sag_bit_flags_input() {
    //set bit flags
    int active_spr = sag_active_sprite_id();
    if (rge_input_state.kb_1.released) { sprite_flags[active_spr][0] = !sprite_flags[active_spr][0]; rge_input_state.kb_1.released = false; }
    else if (rge_input_state.kb_1.released) { sprite_flags[active_spr][0] = !sprite_flags[active_spr][0]; rge_input_state.kb_1.released = false; }
    else if (rge_input_state.kb_2.released) { sprite_flags[active_spr][1] = !sprite_flags[active_spr][1]; rge_input_state.kb_2.released = false; }
    else if (rge_input_state.kb_3.released) { sprite_flags[active_spr][2] = !sprite_flags[active_spr][2]; rge_input_state.kb_3.released = false; }
    else if (rge_input_state.kb_4.released) { sprite_flags[active_spr][3] = !sprite_flags[active_spr][3]; rge_input_state.kb_4.released = false; }
    else if (rge_input_state.kb_5.released) { sprite_flags[active_spr][4] = !sprite_flags[active_spr][4]; rge_input_state.kb_5.released = false; }
    else if (rge_input_state.kb_6.released) { sprite_flags[active_spr][5] = !sprite_flags[active_spr][5]; rge_input_state.kb_6.released = false; }
    else if (rge_input_state.kb_7.released) { sprite_flags[active_spr][6] = !sprite_flags[active_spr][6]; rge_input_state.kb_7.released = false; }
    else if (rge_input_state.kb_8.released) { sprite_flags[active_spr][7] = !sprite_flags[active_spr][7]; rge_input_state.kb_8.released = false; }
}
void sag_use_active_tool_input() {
    //draw based on active tool
    if (rge_input_state.kb_space.released) {
        switch (active_tool) {
        case PENCIL:
            //draw active palette color at index into VRAM texture
            FILL(palette_column + 1);
            RECTFILL(sprites[sag_active_sprite_id()], cursor_column * 2, cursor_row * 2, 1, 1);
            break;
        case FLOOD_FILL:
            sag_flood_fill(PGET(sprites[sag_active_sprite_id()], cursor_column * 2, cursor_row * 2),
                palette_column + 1, cursor_column * 2, cursor_row * 2, sag_active_sprite_id());
            break;
        }

        rge_input_state.kb_space.released = false;
    }
}
void sag_anim_input() {
    
    //NOTE: any input stops animation
    //set anim_end
    if (rge_input_state.kb_comma.released) {
        //END_ID (PREV)
        anim_end = ((anim_end - 1) >= anim_start) ? anim_end - 1 : 255;
        sag_reset_anim();
        rge_input_state.kb_comma.released = false;
    } else if (rge_input_state.kb_period.released) {
        //END_ID (NEXT)
        anim_end = ((anim_end + 1) <= 255) ? anim_end + 1 : anim_start;
        sag_reset_anim();
        rge_input_state.kb_period.released = false;
    //set anim speed
    } else if (rge_input_state.kb_semicolon.released) {
        //ANIM_SPEED (LESS)
        anim_speed = ((anim_speed - 1) >= 1) ? anim_speed - 1 : 30;
        rge_input_state.kb_semicolon.released = false;
    } else if (rge_input_state.kb_single_qoute.released) {
        //ANIM_SPEED (MORE)
        anim_speed = ((anim_speed + 1) <= 30) ? anim_speed + 1 : 1;
        rge_input_state.kb_single_qoute.released = false;
    //set anim_start
    } else if (rge_input_state.kb_open_sqr_bracket.released) {
        //START_ID (PREV)
        anim_start = ((anim_start - 1) >= 0) ? anim_start - 1 : 255;
        anim_end = (anim_start > anim_end) ? anim_start : anim_end;
        sag_reset_anim();
        rge_input_state.kb_open_sqr_bracket.released = false;
    } else if (rge_input_state.kb_close_sqr_bracket.released) {
        //START_ID (NEXT)
        anim_start = ((anim_start + 1) <= 255) ? anim_start + 1 : 0;
        anim_end = (anim_start > anim_end) ? anim_start : anim_end;
        sag_reset_anim();
        rge_input_state.kb_close_sqr_bracket.released = false;
    //set anim_end to active sprite, if its < than anim_start we set to anim_start
    } else if (rge_input_state.kb_slash.released) {
        anim_end = sag_active_sprite_id();
        anim_start = (anim_end < anim_start) ? anim_end : anim_start;
        sag_reset_anim();
        rge_input_state.kb_slash.released = false;
    //set anim_end to active sprite, if its < than anim_start we set to anim_start
    } else if (rge_input_state.kb_backslash.released) {
        anim_start = sag_active_sprite_id();
        anim_end = (anim_start <= anim_end) ? anim_end : sag_active_sprite_id();
        sag_reset_anim();
        rge_input_state.kb_backslash.released = false;
    }
}

//we'll switch this out for a sprite later but for now we'll just use text
void sag_draw_active_tool() {
    TEXT(ui_layer, 200, 20, tool_text[active_tool], SAG_WHITE, SAG_BLACK, TEXT_ALIGN_LEFT);
}

//START OF engine/scene lifecycle methods
void scene_sprite_editor_update() {
    if (rge_input_state.start.released) {
        //manually set the state of exit_game button so it will activate and exit game
        //ui_component_t* entire_text_btn = rge_ui_manager_get_component_by_name("exit_game");
        //entire_text_btn->props.state = ACTIVATED;

        rge_input_state.start.released = false;
        //add editor menu scene, tag with this scene name so it can be restored later
        rge_scene_manager.add_scene(scene_sag_editor_menu_init("sprite_editor"));
    }

    sag_canvas_input();
    sag_move_canvas_input();
    sag_bit_flags_input();
    sag_use_active_tool_input();
    sag_anim_input();

    if (playing_anim) {
        sag_next_anim_preview_sprite();
    }


    //TODO: replace binding with enter
    if (rge_input_state.kb_e.released) {
        playing_anim = true;
        rge_input_state.kb_e.released = false;
    }

    //adjust active sprite page
    if (rge_input_state.kb_pgup.released) {
        sprite_page = (sprite_page > 0) ? sprite_page - 1 : 15;
        rge_input_state.kb_pgup.released = false;
    }
    else if (rge_input_state.kb_pgdn.released) {
        sprite_page = (sprite_page < 15) ? sprite_page + 1 : 0;
        rge_input_state.kb_pgdn.released = false;
    }

    //adjust palette index
    if (rge_input_state.kb_o.released) {
        palette_column = (palette_column > 0) ? palette_column - 1 : 15;
        rge_input_state.kb_o.released = false;
    }
    else if (rge_input_state.kb_p.released) {
        palette_column = (palette_column < 15) ? palette_column + 1 : 0;
        rge_input_state.kb_p.released = false;
    }

    //move sprite selection
    if (rge_input_state.kb_i.released) {
        sprite_row = (sprite_row == 0) ? 1 : 0;
        rge_input_state.kb_i.released = false;
    }
    else if (rge_input_state.kb_j.released) {
        sprite_column = (sprite_column > 0) ? sprite_column - 1 : 7;
        rge_input_state.kb_j.released = false;
    }
    else if (rge_input_state.kb_l.released) {
        sprite_column = (sprite_column < 7) ? sprite_column + 1 : 0;
        rge_input_state.kb_l.released = false;
    }
    else if (rge_input_state.kb_k.released) {
        sprite_row = (sprite_row == 1) ? 0 : 1;
        rge_input_state.kb_k.released = false;
    }

    //change tool
    if (rge_input_state.kb_tab.released) {
        sag_select_next_tool();
        rge_input_state.kb_tab.released = false;
    }

    //flip active sprite vertically or horizontally
    if (rge_input_state.kb_f.released) {
        sag_flip_sprite(sag_active_sprite_id(), true, false);
        rge_input_state.kb_f.released = false;
    }
    else if (rge_input_state.kb_v.released) {
        sag_flip_sprite(sag_active_sprite_id(), false, true);
        rge_input_state.kb_v.released = false;
    }

    //rotate active sprite by 90 degrees
    if (rge_input_state.kb_r.released) {
        sag_rotate_sprite(sag_active_sprite_id());
        rge_input_state.kb_r.released = false;
    }

    //grab color under cursor and set as active color
    if (rge_input_state.kb_q.released) {
        palette_column = PGET(sprites[sag_active_sprite_id()], cursor_column * 2, cursor_row * 2) - 1;
        rge_input_state.kb_q.released = false;
    }

}

void scene_sprite_editor_draw() {
    sag_clear_screen();

    //top ui bar
    sag_draw_ui_bars();
    sag_draw_global_keys();

    sag_draw_sprite_editor_canvas();
    sag_draw_sprite_flags();
    sag_draw_sprite_preview();
    sag_draw_animation_preview();
    sag_draw_current_palette();
    sag_draw_sprite_page(sprite_page);
    sag_draw_active_tool();
}

void scene_sprite_editor_dispose() {
    ui_component_t* tmp = rge_ui_manager_get_component_by_name("exit_game");
    rge_ui_manager.dispose_component(tmp->props.id);
}

scene_t scene_sprite_editor_init() {

    //create a scene
    scene_t scene_sprite_editor;
    rge_scene_init(&scene_sprite_editor, "sprite_editor", 
        scene_sprite_editor_draw, 
        scene_sprite_editor_update, 
        scene_sprite_editor_dispose);

    ui_button_t btn_exit_game;
    rge_ui_button_init(&btn_exit_game, "exit_game", 
        "  PRESS <ENTER> ", 
        "sprite_editor", 
        RGE_BASE_SCREEN_WIDTH / 2, RGE_BASE_SCREEN_HEIGHT / 2 + 24, 
        0, 0, 
        254, 255, 254, 
        NULL, scene_sprite_editor_exit_game);

    rge_ui_manager.add_component(btn_exit_game.component);

    active_tool = PENCIL;

    //set flags for sprites
    for (int i = 0; i < 256; i++) {
        for (int bit = 0; bit < 8; bit++) {  //left to right (hence high order bit 0b10000000 is the rendered properly/left most flag)
            sprite_flags[i][7-bit] = get_byte_flag(flags_for_sprites[i], bit);
        }
    }

    return scene_sprite_editor;
}
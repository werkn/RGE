#include "sag_core.h"

/**
* TODO:
*  - Implement proper palettes based on our requirements
*  - Implement 2BPP/4BPP imprort export
*  - Implement export header
*  - Implement export bmp
*  - implement import bmp
*  - proper way to close out application
*/

void sag_draw_cursor(int x, int y) {

    STROKE(SAG_BLACK);
    LINE(ui_layer, x + 5, y + 5, x + 13, y + 13);
    LINE(ui_layer, x + 5, y + 13, x + 13, y + 5);

    //white with black lines
    STROKE(SAG_WHITE);
    LINE(ui_layer, x + 4, y + 4, x + 12, y + 12);
    LINE(ui_layer, x + 4, y + 12, x + 12, y + 4);
}

void sag_draw_ui_bars() {
    FILL(SAG_PINK);
    RECTFILL(ui_layer, 0, 0, 320, 12);

    FILL(SAG_PINK);
    RECTFILL(ui_layer, 0,228, 320, 12);
}

void sag_draw_global_keys() {
    TEXT(ui_layer, 4, 2, "[F1]Sprites", SAG_WHITE, SAG_PINK, TEXT_ALIGN_LEFT);
    TEXT(ui_layer, 94, 2, "[F2]Map", SAG_BROWN, SAG_PINK, TEXT_ALIGN_LEFT);
    TEXT(ui_layer, 152, 2, "[F3]SFX", SAG_BROWN, SAG_PINK, TEXT_ALIGN_LEFT);
    TEXT(ui_layer, 210, 2, "[F4]SEQ", SAG_BROWN, SAG_PINK, TEXT_ALIGN_LEFT);
}

void sag_clear_screen() {
    CLS();
    FILL(254); //black
    RECTFILL(sprite_layer, 0, 0, 320, 240);
}

void sag_reset_keybd(char* keybd_input, int* keybd_index, int max_length) {
    //reset index
    *keybd_index = 0;
    //wipe string
    snprintf(keybd_input, max_length, "");

    //reset all the keys that will be used so we don't fire keys that were pressed earlier and
    //not cleared
    rge_input_state.kb_a.released = false; 
    rge_input_state.kb_b.released = false; 
    rge_input_state.kb_c.released = false; 
    rge_input_state.kb_d.released = false; 
    rge_input_state.kb_e.released = false; 
    rge_input_state.kb_f.released = false; 
    rge_input_state.kb_g.released = false; 
    rge_input_state.kb_h.released = false; 
    rge_input_state.kb_i.released = false; 
    rge_input_state.kb_j.released = false; 
    rge_input_state.kb_k.released = false; 
    rge_input_state.kb_l.released = false; 
    rge_input_state.kb_m.released = false; 
    rge_input_state.kb_n.released = false; 
    rge_input_state.kb_o.released = false; 
    rge_input_state.kb_p.released = false; 
    rge_input_state.kb_q.released = false; 
    rge_input_state.kb_r.released = false; 
    rge_input_state.kb_s.released = false; 
    rge_input_state.kb_t.released = false; 
    rge_input_state.kb_u.released = false; 
    rge_input_state.kb_v.released = false; 
    rge_input_state.kb_w.released = false; 
    rge_input_state.kb_x.released = false; 
    rge_input_state.kb_y.released = false; 
    rge_input_state.kb_z.released = false; 
    rge_input_state.kb_0.released = false; 
    rge_input_state.kb_1.released = false; 
    rge_input_state.kb_2.released = false; 
    rge_input_state.kb_3.released = false; 
    rge_input_state.kb_4.released = false; 
    rge_input_state.kb_5.released = false; 
    rge_input_state.kb_6.released = false; 
    rge_input_state.kb_7.released = false; 
    rge_input_state.kb_8.released = false; 
    rge_input_state.kb_9.released = false; 
    rge_input_state.kb_backspace.released = false;
    rge_input_state.kb_enter.released = false;
    rge_input_state.kb_esc.released = false;
}

void sag_set_keybd_input_char(char* keybd_input, int* keybd_index, char value, int max_length) {
    
    if (value == '\0') {
        //set current to space
        keybd_input[(*keybd_index)] = '\0';
        keybd_input[((*keybd_index) - 1 < 0) ? 0 : --(*keybd_index)] = '\0';  //delete character
    } else {
        keybd_input[(*keybd_index) + 1 >= max_length ? max_length - 1 : (*keybd_index)++] = value;
        //move cursor indicator to next spot
        keybd_input[(*keybd_index)] = '\0';
    }
}

//ugly implementation but it works for now
void sag_get_keybd_input(char* keybd_input, int* keybd_index, int max_length) {

    //TODO: convert rge_input_state to an array so we can just iterate over it as a for loop
    if (rge_input_state.kb_a.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'a', max_length); rge_input_state.kb_a.released = false; }
    if (rge_input_state.kb_b.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'b', max_length); rge_input_state.kb_b.released = false; }
    if (rge_input_state.kb_c.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'c', max_length); rge_input_state.kb_c.released = false; }
    if (rge_input_state.kb_d.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'd', max_length); rge_input_state.kb_d.released = false; }
    if (rge_input_state.kb_e.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'e', max_length); rge_input_state.kb_e.released = false; }
    if (rge_input_state.kb_f.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'f', max_length); rge_input_state.kb_f.released = false; }
    if (rge_input_state.kb_g.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'g', max_length); rge_input_state.kb_g.released = false; }
    if (rge_input_state.kb_h.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'h', max_length); rge_input_state.kb_h.released = false; }
    if (rge_input_state.kb_i.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'i', max_length); rge_input_state.kb_i.released = false; }
    if (rge_input_state.kb_j.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'j', max_length); rge_input_state.kb_j.released = false; }
    if (rge_input_state.kb_k.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'k', max_length); rge_input_state.kb_k.released = false; }
    if (rge_input_state.kb_l.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'l', max_length); rge_input_state.kb_l.released = false; }
    if (rge_input_state.kb_m.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'm', max_length); rge_input_state.kb_m.released = false; }
    if (rge_input_state.kb_n.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'n', max_length); rge_input_state.kb_n.released = false; }
    if (rge_input_state.kb_o.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'o', max_length); rge_input_state.kb_o.released = false; }
    if (rge_input_state.kb_p.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'p', max_length); rge_input_state.kb_p.released = false; }
    if (rge_input_state.kb_q.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'q', max_length); rge_input_state.kb_q.released = false; }
    if (rge_input_state.kb_r.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'r', max_length); rge_input_state.kb_r.released = false; }
    if (rge_input_state.kb_s.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 's', max_length); rge_input_state.kb_s.released = false; }
    if (rge_input_state.kb_t.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 't', max_length); rge_input_state.kb_t.released = false; }
    if (rge_input_state.kb_u.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'u', max_length); rge_input_state.kb_u.released = false; }
    if (rge_input_state.kb_v.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'v', max_length); rge_input_state.kb_v.released = false; }
    if (rge_input_state.kb_w.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'w', max_length); rge_input_state.kb_w.released = false; }
    if (rge_input_state.kb_x.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'x', max_length); rge_input_state.kb_x.released = false; }
    if (rge_input_state.kb_y.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'y', max_length); rge_input_state.kb_y.released = false; }
    if (rge_input_state.kb_z.released) { sag_set_keybd_input_char(keybd_input, keybd_index, 'z', max_length); rge_input_state.kb_z.released = false; }
    if (rge_input_state.kb_0.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '0', max_length); rge_input_state.kb_0.released = false; }
    if (rge_input_state.kb_1.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '1', max_length); rge_input_state.kb_1.released = false; }
    if (rge_input_state.kb_2.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '2', max_length); rge_input_state.kb_2.released = false; }
    if (rge_input_state.kb_3.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '3', max_length); rge_input_state.kb_3.released = false; }
    if (rge_input_state.kb_4.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '4', max_length); rge_input_state.kb_4.released = false; }
    if (rge_input_state.kb_5.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '5', max_length); rge_input_state.kb_5.released = false; }
    if (rge_input_state.kb_6.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '6', max_length); rge_input_state.kb_6.released = false; }
    if (rge_input_state.kb_7.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '7', max_length); rge_input_state.kb_7.released = false; }
    if (rge_input_state.kb_8.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '8', max_length); rge_input_state.kb_8.released = false; }
    if (rge_input_state.kb_9.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '9', max_length); rge_input_state.kb_9.released = false; }
    if (rge_input_state.kb_backspace.released) { sag_set_keybd_input_char(keybd_input, keybd_index, '\0', max_length); rge_input_state.kb_backspace.released = false; }
}
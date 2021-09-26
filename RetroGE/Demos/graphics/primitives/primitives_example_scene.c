#include "primitives_example_scene.h"

void scene_primitives_example_exit_game(char* args) {
    ui_component_t* exit_game_btn = rge_ui_manager_get_component_by_name("exit_game");
    exit_game_btn->props.state = HIGHLIGHTED;

    //destroy this scene and exit game
    scene_t* tmp = rge_scene_manager_get_scene_by_name("primitives_example");
    rge_scene_manager.dispose_scene(tmp->id);
    rge_shutdown = true;

    //if we want to instead transition to another scene do that here
    //rge_scene_manager.add_scene( ... scene init method );
}

void scene_primitives_example_update() {
    if (rge_input_state.start.released) {
        //manually set the state of exit_game button so it will activate and exit game
        ui_component_t* entire_text_btn = rge_ui_manager_get_component_by_name("exit_game");
        entire_text_btn->props.state = ACTIVATED;

        //for now we manually need to reset released state
        rge_input_state.start.released = false;
    }
}

void scene_primitives_example_draw() {

    //each frame behind the scenes rge_clear is currently called, need to update this to not 
    //be done auto.
    CLS();
    FILL(254);
    RECTFILL(sprite_layer, 0, 0, RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_HEIGHT);

    TEXT(ui_layer, 
        0, 220, 
        "TO EXIT DEMO", 
        255, 0, 
        TEXT_ALIGN_LEFT);

    TEXT(ui_layer, 5, 5, "FILL(#) sets the fill color.",
        255, 0, TEXT_ALIGN_LEFT);
    TEXT(ui_layer, 5, 13, "STROKE(#) sets the border.",
        255, 0, TEXT_ALIGN_LEFT);

    //draw each of the primitives supported by engine
    STROKE(1);
    RECT(sprite_layer, 25, 25, 25, 25);
    TEXT(ui_layer, 55, 30, "RECT(layer,x,y,w,h)", 255, 0, TEXT_ALIGN_LEFT);

    FILL(2);
    RECTFILL(sprite_layer, 25, 55, 25, 25);
    TEXT(ui_layer, 55, 60, "RECTFILL(layer,x,y,w,h)", 255, 0, TEXT_ALIGN_LEFT);

    STROKE(4);
    CIRC(sprite_layer, 25, 85, 12);
    TEXT(ui_layer, 55, 90, "CIRC(layer,x,y,r)", 255, 0, TEXT_ALIGN_LEFT);

    FILL(13);
    CIRCFILL(sprite_layer, 25, 115, 12);
    TEXT(ui_layer, 55, 120, "CIRCFILL(layer,x,y,r)", 255, 0, TEXT_ALIGN_LEFT);

    STROKE(88);
    LINE(sprite_layer, 25, 145, 50, 170);
    TEXT(ui_layer, 55, 150, "LINE(layer,x1,y1,x2,y2)", 255, 0, TEXT_ALIGN_LEFT);

    FILL(88);
    TRIFILL(sprite_layer, 25, 175, 38, 200, 50, 175);
    TEXT(ui_layer, 55, 180, "TRIFILL(layer,x1,y1,x2,y2,x3,y3)", 255, 0, TEXT_ALIGN_LEFT);

    STROKE(48);
    TRI(sprite_layer, 25, 175, 38, 200, 50, 175);
    TEXT(ui_layer, 65, 190, "TRI(layer,x1,y1,x2,y2,x3,y3)", 255, 0, TEXT_ALIGN_LEFT);
}

void scene_primitives_example_dispose() {
    ui_component_t* tmp = rge_ui_manager_get_component_by_name("exit_game");
    rge_ui_manager.dispose_component(tmp->props.id);
}

scene_t scene_primitives_example_init() {
    
    //create a scene
    scene_t scene_primitives_example;
    rge_scene_init(&scene_primitives_example, "primitives_example", 
        scene_primitives_example_draw, 
        scene_primitives_example_update, 
        scene_primitives_example_dispose);

    ui_button_t btn_exit_game;
    rge_ui_button_init(&btn_exit_game, "exit_game", 
        "  PRESS <ENTER> ", 
        "primitives_example", 
        50, 230, 
        0, 0, 
        254, 255, 254, 
        NULL, scene_primitives_example_exit_game);

    rge_ui_manager.add_component(btn_exit_game.component);

    return scene_primitives_example;
}
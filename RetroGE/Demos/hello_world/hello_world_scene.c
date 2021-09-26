#include "hello_world_scene.h"

void scene_hello_world_exit_game(char* args) {
    ui_component_t* exit_game_btn = rge_ui_manager_get_component_by_name("exit_game");
    exit_game_btn->props.state = HIGHLIGHTED;

    //destroy this scene and exit game
    scene_t* tmp = rge_scene_manager_get_scene_by_name("hello_world");
    rge_scene_manager.dispose_scene(tmp->id);
    rge_shutdown = true;

    //if we want to instead transition to another scene do that here
    //rge_scene_manager.add_scene( ... scene init method );
}

void scene_hello_world_update() {
    if (rge_input_state.start.released) {
        //manually set the state of exit_game button so it will activate and exit game
        ui_component_t* entire_text_btn = rge_ui_manager_get_component_by_name("exit_game");
        entire_text_btn->props.state = ACTIVATED;

        //for now we manually need to reset released state
        rge_input_state.start.released = false;
    }
}

void scene_hello_world_draw() {

    RECT(sprite_layer, 0, 0, 24, 24);
   
    TEXT(ui_layer, 
        RGE_BASE_SCREEN_WIDTH/2, RGE_BASE_SCREEN_HEIGHT/2, 
        "~~HELLO WORLD!~~", 
        255, 0, 
        TEXT_ALIGN_CENTER);
}

void scene_hello_world_dispose() {
    ui_component_t* tmp = rge_ui_manager_get_component_by_name("exit_game");
    rge_ui_manager.dispose_component(tmp->props.id);
}

scene_t scene_hello_world_init() {
    
    //create a scene
    scene_t scene_hello_world;
    rge_scene_init(&scene_hello_world, "hello_world", 
        scene_hello_world_draw, 
        scene_hello_world_update, 
        scene_hello_world_dispose);

    ui_button_t btn_exit_game;
    rge_ui_button_init(&btn_exit_game, "exit_game", 
        "  PRESS <ENTER> ", 
        "hello_world", 
        RGE_BASE_SCREEN_WIDTH / 2, RGE_BASE_SCREEN_HEIGHT / 2 + 24, 
        0, 0, 
        254, 255, 254, 
        NULL, scene_hello_world_exit_game);

    rge_ui_manager.add_component(btn_exit_game.component);

    return scene_hello_world;
}
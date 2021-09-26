#include "pixels_example_scene.h"

void scene_pixels_example_exit_game(char* args) {
    ui_component_t* exit_game_btn = rge_ui_manager_get_component_by_name("exit_game");
    exit_game_btn->props.state = HIGHLIGHTED;

    //destroy this scene and exit game
    scene_t* tmp = rge_scene_manager_get_scene_by_name("pixels_example");
    rge_scene_manager.dispose_scene(tmp->id);
    rge_shutdown = true;

    //if we want to instead transition to another scene do that here
    //rge_scene_manager.add_scene( ... scene init method );
}

void scene_pixels_example_update() {
    if (rge_input_state.start.released) {
        //manually set the state of exit_game button so it will activate and exit game
        ui_component_t* entire_text_btn = rge_ui_manager_get_component_by_name("exit_game");
        entire_text_btn->props.state = ACTIVATED;

        //for now we manually need to reset released state
        rge_input_state.start.released = false;
    }
}

void scene_pixels_example_draw() {

    //each frame behind the scenes rge_clear is currently called, need to update this to not 
    //be done auto. and instead only occur on CLS()
    CLS();
    FILL(254);
    RECTFILL(sprite_layer, 0, 0, RGE_BASE_SCREEN_WIDTH, RGE_BASE_SCREEN_HEIGHT);

    TEXT(ui_layer,
        0, 220,
        "TO EXIT DEMO",
        255, 0,
        TEXT_ALIGN_LEFT);

    //draw simulated noise
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            int color = 250 + RAND(5);
            PSET(sprite_layer, x, y, color);
        }
    }

    //use PGET to get color if pixel @ x,y
    char tmp[16];
    int millis = MILLIS();
    int sec = SECONDS();
    int min = MINUTES();

    //check our timing methods
    snprintf(tmp, 16, "G: %d", global_timer_tick);
    TEXT(ui_layer, 0, 170, tmp, 254, 255, TEXT_ALIGN_LEFT);
    
    snprintf(tmp, 16, "MS: %d", millis);
    TEXT(ui_layer, 0, 180, tmp, 254, 255, TEXT_ALIGN_LEFT);

    snprintf(tmp, 16, "S: %d M: %d", sec, min);
    TEXT(ui_layer, 0, 190, tmp, 254, 255, TEXT_ALIGN_LEFT);

    TEXT(ui_layer, 160, 120, "  NO SIGNAL  ", 254, 255, TEXT_ALIGN_CENTER);
}

void scene_pixels_example_dispose() {
    ui_component_t* tmp = rge_ui_manager_get_component_by_name("exit_game");
    rge_ui_manager.dispose_component(tmp->props.id);
}

scene_t scene_pixels_example_init() {
    
    //create a scene
    scene_t scene_pixels_example;
    rge_scene_init(&scene_pixels_example, "pixels_example", 
        scene_pixels_example_draw, 
        scene_pixels_example_update, 
        scene_pixels_example_dispose);

    ui_button_t btn_exit_game;
    rge_ui_button_init(&btn_exit_game, "exit_game",
        "  PRESS <ENTER> ",
        "pixels_example",
        50, 230,
        0, 0,
        254, 255, 254,
        NULL, scene_pixels_example_exit_game);

    rge_ui_manager.add_component(btn_exit_game.component);

    return scene_pixels_example;
}
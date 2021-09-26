#include "../SAG Editor/Sprite Editor/sprite_editor_scene.h"

int main()
{
    rge_state_t init_state = rge_init(RESOLUTION_640x480, "../SAG Editor/Assets/editor.sag");
    if (init_state != RGE_STATE_OK) { assert("Error setting up rge." && false); }

    //DO NOT PUT ANY CODE BEFORE THIS LINE

    //add starting scene
    rge_scene_manager.add_scene(scene_sprite_editor_init());

    while (!rge_shutdown) {

        if (input_tick) {
            //update input state
            rge_input();

            //START INPUT CALLS

            //>>>> PUT ANY INPUT CALLS TO OVERRIDE SCENE HANDLING HERE <<<<
            
            //END INPUT CALLS

            //reset input loop flag
            input_tick = false;
        }

        if (frame_tick) {
            rge_gfx.clear();
            //START DRAW CALLS

            //>>>> PUT ANY DRAW CALLS OUTSIDE OF SCENE MGMT SYSTEM HERE <<<<
         
            //END DRAW CALLS
            rge_gfx.flip_backbuffer();
        }

        rge_update();
    }

    //>>>> PUT ANY CUSTOM DISPOSE / CLEANUP CODE HERE <<<<

    //RELEASE ANY CUSTOM ALLOC BEFORE THIS LINE
    rge_dispose();
    return 0;
}
END_OF_MAIN()
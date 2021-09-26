#include "../Demos/graphics/primitives/primitives_example_scene.h"
#include "../Demos/graphics/sprites/sprites_example_scene.h"
#include "../Demos/graphics/pixels/pixels_example_scene.h"


int main()
{
    rge_state_t init_state = rge_init(RESOLUTION_640x480, "./Demos/demo_assets/sprites.bmp");
    if (init_state != RGE_STATE_OK) { assert("Error setting up rge." && false); }

    //DO NOT PUT ANY CODE BEFORE THIS LINE

    //add starting scene
    rge_scene_manager.add_scene(scene_pixels_example_init());

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
// render.h

#include "scene.h"

/*
 *
 *  Static Functions
 *
 */

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void render_scene(scene* s);

void render_lighting(scene* s);

void render_applyCamera(camera* cam);

// Initialise the 3D rendering
void render_init();

// Terminate the 3D rendering
void render_terminate();

// Render the current scene
// This is where the business happens
void render(scene* s);

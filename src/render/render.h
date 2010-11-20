// render.h

#include "scene.h"

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void render_scene(scene* s);

void render_lighting(scene* s);

void render_applyCamera(vector* camera);

// Initialise the 3D rendering
void render_init();

// Render the current scene
// This is where the business happens
void render(scene* s);

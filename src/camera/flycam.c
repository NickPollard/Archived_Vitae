// flycam.c

flycam* flycam_create() {
	flycam* f = mem_alloc(sizeof(flycam));
	f->pan_sensitivity = Vector(1.f, 1.f, 1.f, 0.f);
	f->track_sensitivity = Vector(1.f, 1.f, 1.f, 0.f);
	return f;
}


// Read in an input structure
void flycam_input(flycam* cam, flycam_input* in) {
	vector translation = matrixVecMul(cam->transform, in->track);
	matrix rotation = matrixFromEulerAngles(in->pan);
	// Rotate the flycam by the pan input
	cam->transform = matrixMul(cam->transform, rotation);
	// Translate the flycam by the track inpuT
	cam->transform = matrixTranslationAdd(cam->transform, translation);	
}

// Set the camera target to output frame data to
void flycam_setTarget(flycam* f, camera* c) {
	f->camera_target = c;
}

// Update the flycam, setting the target data to latest
void flycam_update(flycam* f, float dt) {
	transform_setWorldMatrix(camera_target->transform, f->transform);
}

// gamecamera.h

typedef struct gamecamera_input_s {
	Vec3 translation;
	Vec3 rotation;
} gamecamera_input;

typedef struct gamecamera_s {
	camera* camera;
	gamecamera_input input;
} gamecamera;


#ifndef __COMMON_FWD_H__
#define __COMMON_FWD_H__

struct heapAllocator_s;
struct debugtextframe_s;
struct input_s;
struct camera_s;
struct light_s;
struct model_s;
struct modelInstance_s;
struct transform_s;
struct mesh_s;
struct scene_s;
union vector_u;

typedef struct heapAllocator_s heapAllocator;
typedef struct camera_s camera;
typedef struct debugtextframe_s debugtextframe;
typedef struct input_s input;
typedef struct light_s light;
typedef struct model_s model;
typedef struct modelInstance_s modelInstance;
typedef struct mesh_s mesh;
typedef struct transform_s transform;
typedef union vector_u vector;
typedef union vector_u color;
typedef struct scene_s scene;

typedef int modelHandle; // A Handle into the model array


#endif // __COMMON_FWD_H__

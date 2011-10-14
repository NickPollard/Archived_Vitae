#ifndef __COMMON_FWD_H__
#define __COMMON_FWD_H__

struct camera_s;
struct debugtextframe_s;
struct engine_s;
struct heapAllocator_s;
struct input_s;
struct light_s;
struct map_s;
struct model_s;
struct modelInstance_s;
struct mesh_s;
struct particleEmitter_s;
struct scene_s;
struct shader_s;
struct transform_s;
struct vertex_s;
union vector_u;

typedef struct camera_s camera;
typedef struct debugtextframe_s debugtextframe;
typedef struct engine_s engine;
typedef struct heapAllocator_s heapAllocator;
typedef struct input_s input;
typedef struct light_s light;
typedef struct map_s map;
typedef struct model_s model;
typedef struct modelInstance_s modelInstance;
typedef struct mesh_s mesh;
typedef struct particleEmitter_s particleEmitter;
typedef struct scene_s scene;
typedef struct shader_s shader;
typedef struct transform_s transform;
typedef struct vertex_s vertex;

typedef union vector_u vector;
typedef union vector_u color;

typedef int modelHandle; // A Handle into the model array


#endif // __COMMON_FWD_H__

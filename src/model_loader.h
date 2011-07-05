// model_loader.h

#pragma once

#define MAX_OBJ_VERTICES 256
#define MAX_OBJ_NORMALS  256
#define MAX_OBJ_INDICES 1024

model* LoadObj( const char* filename );

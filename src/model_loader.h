// model_loader.h

#pragma once

#define kObjMaxVertices 64 << 10
#define kObjMaxIndices 128 << 10

model* model_load( const char* filename );

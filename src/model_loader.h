// model_loader.h

#pragma once

#define kObjMaxVertices 64 << 10
#define kObjMaxIndices 128 << 10

model* LoadObj( const char* filename );

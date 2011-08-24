// model_loader.h

#pragma once

#define kObjMaxVertices 1024
#define kObjMaxNormals  1024
#define kObjMaxIndices 2048

model* LoadObj( const char* filename );

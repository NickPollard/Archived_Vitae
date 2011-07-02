#include "common.h"
#include "model_loader.h"
//--------------------------------------------------------
#include "system/file.h"
#include "system/string.h"
#include "scene.h"
#include "model.h"


model* LoadObj( const char* filename ) {
	// Load the raw data
	int vertCount = 0, indexCount = 0;
	vector	vertices[MAX_OBJ_VERTICES];
	int		indices[MAX_OBJ_INDICES];

	int file_length;
	char* file_buffer = vfile_contents( filename, &file_length );
	inputStream* stream = inputStream_create( file_buffer );

	while ( !inputStream_endOfFile( stream )) {
		char* token = inputStream_nextToken( stream );
		if ( string_equal( token, "v" )) {
			// Vertex
			for ( int i = 0; i < 3; i++ ) {
				free( token );
				token = inputStream_nextToken( stream );
				vertices[vertCount].val[i] = strtof( token, NULL );
			}
			printf( " Vertex: %.2f %.2f %.2f \n", vertices[vertCount].val[0], vertices[vertCount].val[1], vertices[vertCount].val[2] );
			vertCount++;
		}
		if ( string_equal( token, "f" )) {
			// Face (indices)
			for ( int i = 0; i < 3; i++ ) {
				free( token );
				token = inputStream_nextToken( stream );
				indices[indexCount++] = atoi( token );
			}
			printf( " Face: %d %d %d \n", indices[indexCount-3], indices[indexCount-2], indices[indexCount-1] );
		}
		inputStream_nextLine( stream );
	}

	printf( "Parsed .obj file with %d verts and %d faces.\n", vertCount, indexCount / 3 );

	// Create the Vitae Model
	model* mdl = model_createModel( 1 ); // Only one mesh by default
	mesh* msh = mesh_createMesh( vertCount, indexCount );
	mdl->meshes[0] = msh;

	// Copy our loaded data into the Mesh structure
	memcpy( msh->verts, vertices, vertCount * sizeof( vector ));
	memcpy( msh->indices, indices, indexCount * sizeof( int ));

	return mdl;
}
/*
// Load an .obj file into Manta
model* LoadObj( const char* filename, vector* objPosition )
{
	byte* buffer;
	byte  lineBuffer[MAX_FILE_LINE_LENGTH];
	bool  reachedEnd = false;
	byte* position;

	if (LoadFile(&buffer, filename))
	{
		int vertexCount = 0;
		int polyCount = 0;
		// Set vertex offset (-1 since obj is indexed from 1)
		vertexOffset = scene->getVertexCount() - 1;
		polyOffset = scene->getPolyCount();
		// Loop through each line
		position = buffer;
		while (!reachedEnd)
		{
			reachedEnd = ReadLine(&position, lineBuffer);
			switch (lineBuffer[0])
			{
				case 'v':
					vertexCount++;
					LoadObj_AddVertex(lineBuffer);
					break;
				case 'f':
					polyCount++;
					LoadObj_AddPolygon(lineBuffer);
					break;
				default:
//					printf("-- Skipping line --\n");
					break;
			}
		}
		free(buffer);
		return scene->addModel(objPosition, vertexOffset+1, vertexCount, polyOffset, polyCount);
	}
	return NULL;
}

vector LoadObj_AddVertex(const char* line)
{
	vector vertex;
	const char* position = line+2;
	byte wordBuffer[MAX_FILE_WORD_LENGTH];
	ReadWord(&position, wordBuffer);
	vertex.x = atof(wordBuffer);
	ReadWord(&position, wordBuffer);
	vertex.y = atof(wordBuffer);
	ReadWord(&position, wordBuffer);
	vertex.z = atof(wordBuffer);
	vertex.w = 1.f;

	return vector;
}
void LoadObj_AddPolygon(const char* line)
{
	int	val[3];
	const char* position = line+2;
	byte wordBuffer[MAX_FILE_WORD_LENGTH];
	ReadWord(&position, wordBuffer);
	val[0] = atoi(wordBuffer) + vertexOffset;
	ReadWord(&position, wordBuffer);
	val[1] = atoi(wordBuffer) + vertexOffset;
	ReadWord(&position, wordBuffer);
	val[2] = atoi(wordBuffer) + vertexOffset;

	indicesAppend( val );
}*/

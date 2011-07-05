#include "common.h"
#include "model_loader.h"
//--------------------------------------------------------
#include "system/file.h"
#include "system/string.h"
#include "scene.h"
#include "model.h"


model* LoadObj( const char* filename ) {
	// Load the raw data
	int vert_count = 0, index_count = 0, normal_count = 0;
	vector	vertices[MAX_OBJ_VERTICES];
	vector	normals[MAX_OBJ_NORMALS];
	int		indices[MAX_OBJ_INDICES];
	int		normal_indices[MAX_OBJ_INDICES];

	// Initialise to 0;
	memset( vertices, 0, sizeof( vector ) * MAX_OBJ_VERTICES );
	memset( normals, 0, sizeof( vector ) * MAX_OBJ_NORMALS );
	memset( indices, 0, sizeof( int ) * MAX_OBJ_INDICES );

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
				vertices[vert_count].val[i] = strtof( token, NULL );
			}
			//printf( " Vertex: %.2f %.2f %.2f \n", vertices[vert_count].val[0], vertices[vert_count].val[1], vertices[vert_count].val[2] );
			vert_count++;
		}
		if ( string_equal( token, "vn" )) {
			// Vertex Normal
			for ( int i = 0; i < 3; i++ ) {
				free( token );
				token = inputStream_nextToken( stream );
				normals[normal_count].val[i] = strtof( token, NULL );
			}
			//printf( " Normal: %.2f %.2f %.2f \n", normals[normal_count].val[0], normals[normal_count].val[1], normals[vert_count].val[2] );
			normal_count++;
		}
		if ( string_equal( token, "f" )) {
			// Face (indices)
			for ( int i = 0; i < 3; i++ ) {
				free( token );
				token = inputStream_nextToken( stream );
				// Need to split into 3 parts (vert/tex/normal) by /
				int len = strlen( token );
				char vert[8];
				char uv[8];
				char norm[8];
				int i = 0;
				const char* string = token;
				while ( string[i] != '/' && i < len ) {
					vert[i] = string[i];
					i++;
				}
				vert[i] = '\0';
				string = &string[i+1];
				i = 0;
				while ( string[i] != '/' && i < len ) {
					uv[i] = string[i];
					i++;
				}
				uv[i] = '\0';
				string = &string[i+1];
				i = 0;
				while ( string[i] != '/' && i < len ) {
					norm[i] = string[i];
					i++;
				}
				norm[i] = '\0';

//				//printf( "Vert: %s, UV: %s, Norm %s.\n", vert, uv, norm );

				indices[index_count] = atoi( vert ) - 1; // -1 as obj uses 1-based indices, not 0-based as we do
				normal_indices[index_count] = atoi( norm ) - 1; // -1 as obj uses 1-based indices, not 0-based as we do
				index_count++;
			}
			//printf( " Face: %d %d %d \n", indices[index_count-3], indices[index_count-2], indices[index_count-1] );
		}
		inputStream_nextLine( stream );
	}

	printf( "Parsed .obj file \"%s\" with %d verts and %d faces.\n", filename, vert_count, index_count / 3 );

	// Create the Vitae Model
	model* mdl = model_createModel( 1 ); // Only one mesh by default
	mesh* msh = mesh_createMesh( vert_count, index_count, index_count / 3 /* For now, one normal per face */ );
	mdl->meshes[0] = msh;

	// Copy our loaded data into the Mesh structure
	memcpy( msh->verts, vertices, vert_count * sizeof( vector ));
	memcpy( msh->indices, indices, index_count * sizeof( int ));
	memcpy( msh->normals, normals, normal_count * sizeof( vector ));
	memcpy( msh->normal_indices, normal_indices, index_count * sizeof( int ));

//	mesh_calculateNormals( msh );

	return mdl;
}

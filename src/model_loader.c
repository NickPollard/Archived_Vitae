#include "common.h"
#include "model_loader.h"
//--------------------------------------------------------
#include "system/file.h"
#include "system/string.h"
#include "scene.h"
#include "model.h"

model* LoadObj( const char* filename ) {
	// Load the raw data
	int vert_count = 0, index_count = 0, normal_count = 0, uv_count = 0;
	// Lets create these arrays on the heap, as they need to be big
	// TODO: Could make these static perhaps?
	/*
	vector	vertices[kObjMaxVertices];
	vector	normals[kObjMaxVertices];
	vector	uvs[kObjMaxVertices];
	uint16_t		indices[kObjMaxIndices];
	int		normal_indices[kObjMaxIndices];
	int		uv_indices[kObjMaxIndices];
	*/

	printf( "MODEL_LOADER: Allocating data buffers.\n" );
	vector* vertices = mem_alloc( sizeof( vector ) * kObjMaxVertices );
	vector* normals = mem_alloc( sizeof( vector ) * kObjMaxVertices );
	vector* uvs = mem_alloc( sizeof( vector ) * kObjMaxVertices );
	uint16_t* indices = mem_alloc( sizeof( uint16_t ) * kObjMaxIndices );
	uint16_t* normal_indices = mem_alloc( sizeof( uint16_t ) * kObjMaxIndices );
	uint16_t* uv_indices = mem_alloc( sizeof( uint16_t ) * kObjMaxIndices );

#define array_clear( array, size ) \
	memset( array, 0, sizeof( array[0] ) * size );

	// Initialise to 0;
	/*
	memset( vertices, 0, sizeof( vertices[0] ) * kObjMaxVertices );
	memset( normals, 0, sizeof( normals[0] ) * kObjMaxVertices );
	memset( uvs, 0, sizeof( uvs[0] ) * kObjMaxVertices );
	memset( indices, 0, sizeof( uint16_t ) * kObjMaxIndices );
	memset( normal_indices, 0, sizeof( uint16_t ) * kObjMaxIndices );
	memset( uv_indices, 0, sizeof( uint16_t ) * kObjMaxIndices );
	*/

	array_clear( vertices, kObjMaxVertices );
	array_clear( normals, kObjMaxVertices );
	array_clear( uvs, kObjMaxVertices );
	array_clear( indices, kObjMaxIndices );
	array_clear( normal_indices, kObjMaxIndices );
	array_clear( uv_indices, kObjMaxIndices );

	int file_length;
	char* file_buffer = vfile_contents( filename, &file_length );
	inputStream* stream = inputStream_create( file_buffer );

	while ( !inputStream_endOfFile( stream )) {
		char* token = inputStream_nextToken( stream );
		if ( string_equal( token, "v" )) {
			assert( vert_count < kObjMaxVertices );
			// Vertex
			for ( int i = 0; i < 3; i++ ) {
				mem_free( token );
				token = inputStream_nextToken( stream );
				vertices[vert_count].val[i] = strtof( token, NULL );
			}
			vertices[vert_count].coord.w = 1.f; // Force 1.0 w value for all vertices.
//			printf( " Vertex: %.2f %.2f %.2f \n", vertices[vert_count].val[0], vertices[vert_count].val[1], vertices[vert_count].val[2] );
			vert_count++;
		}
		if ( string_equal( token, "vn" )) {
			assert( normal_count < kObjMaxVertices );
			// Vertex Normal
			for ( int i = 0; i < 3; i++ ) {
				mem_free( token );
				token = inputStream_nextToken( stream );
				normals[normal_count].val[i] = strtof( token, NULL );
			}
			normals[normal_count].coord.w = 0.f; // Force 0.0 w value for all normals
//			printf( " Normal: %.2f %.2f %.2f \n", normals[normal_count].val[0], normals[normal_count].val[1], normals[vert_count].val[2] );
			normal_count++;
		}
		if ( string_equal( token, "vt" )) {
			assert( uv_count < kObjMaxVertices );
			// Vertex Texture Coord (UV)
			for ( int i = 0; i < 2; i++ ) {
				mem_free( token );
				token = inputStream_nextToken( stream );
				uvs[uv_count].val[i] = strtof( token, NULL );
			}
//			printf( " Uv:  %.2f %.2f \n", uvs[uv_count].val[0], uvs[uv_count].val[1] );
			uv_count++;
		}
		if ( string_equal( token, "f" )) {
			// Face (indices)
			for ( int i = 0; i < 3; i++ ) {
				assert( index_count < kObjMaxIndices );

				mem_free( token );
				token = inputStream_nextToken( stream );
				// Need to split into 3 parts (vert/tex/normal) by /
				int len = strlen( token );
				char vert[8];
				char uv[8];
				char norm[8];
				int i = 0;
				const char* string = token;
				while ( string[i] != '/' && i < len ) {
					assert( i >= 0 );
					assert( i < 8 );
					vert[i] = string[i];
					i++;
				}
				vert[i] = '\0';
				string = &string[i+1];
				i = 0;
				len = strlen( string );
				while ( string[i] != '/' && i < len ) {
					assert( i >= 0 );
					assert( i < 8 );
					uv[i] = string[i];
					i++;
				}
				uv[i] = '\0';
				string = &string[i+1];
				i = 0;
				len = strlen( string );
				while ( string[i] != '/' && i < len ) {
					assert( i >= 0 );
					assert( i < 8 );
					norm[i] = string[i];
					i++;
				}
				norm[i] = '\0';

				//printf( "Vert: %s, UV: %s, Norm %s.\n", vert, uv, norm );

				indices[index_count] = atoi( vert ) - 1; // -1 as obj uses 1-based indices, not 0-based as we do
				normal_indices[index_count] = atoi( norm ) - 1; // -1 as obj uses 1-based indices, not 0-based as we do
				uv_indices[index_count] = atoi( uv ) - 1; // -1 as obj uses 1-based indices, not 0-based as we do
				index_count++;
			}
			//printf( " Face: %d %d %d \n", indices[index_count-3], indices[index_count-2], indices[index_count-1] );
		}
		mem_free( token );
		inputStream_nextLine( stream );
	}

	printf( "Parsed .obj file \"%s\" with %d verts and %d faces.\n", filename, vert_count, index_count / 3 );
	printf( "Parsed .obj file \"%s\" with %d normals and %d uvs.\n", filename, normal_count, uv_count );

	// Create the Vitae Model
	model* mdl = model_createModel( 1 ); // Only one mesh by default
	mesh* msh = mesh_createMesh( vert_count, index_count, index_count, uv_count );
	mdl->meshes[0] = msh;

	// Copy our loaded data into the Mesh structure
	memcpy( msh->verts, vertices, vert_count * sizeof( vector ));
	memcpy( msh->indices, indices, index_count * sizeof( uint16_t ));
	memcpy( msh->normals, normals, normal_count * sizeof( vector ));
	memcpy( msh->normal_indices, normal_indices, index_count * sizeof( uint16_t ));
	memcpy( msh->uvs, uvs, uv_count * sizeof( vector ));
	memcpy( msh->uv_indices, uv_indices, index_count * sizeof( uint16_t ));

	mesh_buildBuffers( msh );

	mem_free( file_buffer );

	return mdl;
}

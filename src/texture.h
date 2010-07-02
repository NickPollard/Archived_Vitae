#ifndef __TEXTURE_H__
#define __TEXTURE_H__

// GTK headers
#include <gtk/gtk.h>

typedef GdkPixbuf* TextureHandle;
typedef const char * TextureID;

// Defines
#define TEXTURES_LOADED_MAX 256

typedef struct _texInfo {
	TextureID id;
	TextureHandle handle;
} TextureInfo;

typedef struct _texLib {
	TextureInfo loadedTextures[TEXTURES_LOADED_MAX];
	int count;
} TextureLibrary;

//
// Functions
//


// Loads the texture with TextureID id if not already loaded
// Else returns the existing TextureHandle
// FUTURE: Do reference counting someday?
TextureHandle texture_load_by_id(TextureLibrary* lib, TextureID id);

// Finds the texture handle to the given TextureID
// If not already loaded, returns NULL
TextureHandle texture_find_by_id(TextureLibrary* lib, TextureID id);

// Create and initialise (to zero) a new TextureLibrary
TextureLibrary* texture_library_create();

void texture_library_add_texture(TextureLibrary* lib, TextureID id, TextureHandle handle);
#endif // __TEXTURE_H__

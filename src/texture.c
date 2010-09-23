#include "src/common.h"
#include "src/texture.h"

// Loads the texture with TextureID id if not already loaded
// Else returns the existing TextureHandle
// FUTURE: Do reference counting someday?
TextureHandle texture_load_by_id(TextureLibrary* lib, TextureID id) {
	TextureHandle t = NULL;

	// See if it's already loaded
	t = texture_find_by_id(lib, id);

	// If it's not already loaded, load it
	if (!t)	{
		printf("Texture -- Loading new pixbuf (\"%s\")\n", id);
		// Load the pixbuf
		GError* error = NULL;
//		t = gdk_pixbuf_new_from_file(id, &error);
		(void)error;
		if (!t)
			fprintf(stderr, "Texture -- Error trying to load pixbuf: %s\n", id);
		texture_library_add_texture(lib, id, t);
	}
	return t;
}

// Finds the texture handle to the given TextureID
// If not already loaded, returns NULL
TextureHandle texture_find_by_id(TextureLibrary* lib, TextureID id) {
	printf("Texture --  Searching for already loaded texture %s.\n", id);
	for (int i = 0; i < lib->count; i++) {
		if (lib->loadedTextures[i].id == id) {
			return lib->loadedTextures[i].handle;
			printf("Texture --  Found already loaded texture %s.\n", id);
		}
	}
	return NULL;
}

TextureLibrary* texture_library_create() {
	TextureLibrary* lib = malloc(sizeof(TextureLibrary));
	memset(lib, 0, sizeof(TextureLibrary));
	return lib;
}

void texture_library_add_texture(TextureLibrary* lib, TextureID id, TextureHandle handle) {
	lib->loadedTextures[lib->count].id = id;
	lib->loadedTextures[lib->count].handle = handle;
	lib->count++;
}

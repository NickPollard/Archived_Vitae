// keyboard.c
#include "common.h"
#include "keyboard.h"
//---------------------
#include "input.h"
#include "test.h"

//
// *** keyboard
//

// *** Internal private functions, where they key has been translated into a keycode
int keyCode( enum key k ) {
	return key_codes[k];
}

int keyHeldInternal( input* in, int key_code ) {
	return in->data[in->active].keys.keys[key_code / 8] & (0x1 << (key_code % 8));
}
int keyWasHeldInternal( input* in, int key_code ) {
	return in->data[1 - in->active].keys.keys[key_code / 8] & (0x1 << (key_code % 8));
}

// *** External keyboard accessors

// Is the key held down this frame? Regardless of previous state
int input_keyHeld( input* in, enum key k ) {
	return keyHeldInternal( in, keyCode( k ));
}

// Was the key held down last frame? Regardless of previous state
int input_keyWasHeld( input* in, enum key k ) {
	return keyWasHeldInternal( in, keyCode( k ));
}

// Was the key first pressed this frame? ie. It is depressed now, but was not last frame
int input_keyPressed( input* in, enum key k ) {
	int key_code = keyCode( k );
	return keyHeldInternal( in, key_code ) && !keyWasHeldInternal( in, key_code );
}

// Was the key first released this frame? ie. It is not depressed now, but was last frame
int input_keyReleased( input* in, enum key k ) {
	int key_code = keyCode( k );
	return !keyHeldInternal( in, key_code) && keyWasHeldInternal( in, key_code );
}

void keyArray_set( key_array* keys, int key, int state ) {
	// only two permissible options for state
	vAssert( state == 0x0 || state == 0x1 );
	if ( state )
		keys->keys[ key / 8 ] |= ( 0x1 << ( key % 8 ));
	else
		keys->keys[ key / 8 ] &= ~( 0x1 << ( key % 8 ));

}

int keyArray_get( key_array* keys, int key ) {
	return (keys->keys[ key / 8 ] & ( 0x1 << ( key % 8 ))) != 0;
}

#ifdef LINUX_X
void input_xKeyPress( int key ) {
	keyArray_set( &x_key_array, key, 1);
}
void input_xKeyRelease( int key ) {
	keyArray_set( &x_key_array, key, 0);
}
#endif // LINUX_X

#if UNIT_TEST
void test_keyboard() {
	// Key array test
	key_array data;
	memset( &data, 0, sizeof( key_array ));

	// Test setting key arrays
	for ( int key = 0; key < 256; ++key ) {
		vAssert( keyArray_get( &data, key ) == 0 );
		keyArray_set( &data, key, 1 );
		vAssert( keyArray_get( &data, key ) == 1)
		keyArray_set( &data, key, 0 );
		vAssert( keyArray_get( &data, key ) == 0);
	}
	test( true, "Read and Wrote to all keys in key array successfully.", NULL );
}
#endif // UNIT_TEST

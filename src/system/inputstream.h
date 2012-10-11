// inputstream.h
#pragma once
#include "system/file.h"

inputStream*	inputStream_create( const char* source );
char*	inputStream_nextToken( inputStream* stream );
void	inputStream_freeToken( inputStream* stream, const char* token );
void	inputStream_skipToken( inputStream* stream );
bool	inputStream_endOfFile( inputStream* in );
void	inputStream_nextLine( inputStream* in );
void	inputStream_skipPast( inputStream* stream, const char* string );


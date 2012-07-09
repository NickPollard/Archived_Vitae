// test.c
#include "common.h"
#include "test.h"
//---------------------

#if UNIT_TEST

void test( bool test, char* msg_success, char* msg_fail ) {
	if ( test ) {
		printf( "[ %sPassed%s ]\t%s\n", TERM_GREEN, TERM_WHITE, msg_success );
	}
	else {
		printf( "[ %sFailed%s ]\t%s\n", TERM_RED, TERM_WHITE, msg_fail );
	}
}

#endif // UNIT_TEST

// modelinstance.c

#include "common.h"
#include "modelinstance.h"
//-----------------------
#include "model.h"
#include "transform.h"

IMPLEMENT_POOL( modelInstance )

pool_modelInstance* static_modelInstance_pool = NULL;

void modelInstance_initPool() {
	static_modelInstance_pool = pool_modelInstance_create( 256 );
}

modelInstance* modelInstance_create( modelHandle m ) {
//	modelInstance* i = malloc( sizeof( modelInstance ));
	modelInstance* i = pool_modelInstance_allocate( static_modelInstance_pool );
	i->model = m;
	return i;
}

void modelInstance_draw( modelInstance* instance ) {
	glPushMatrix(); {
		glMultMatrixf( matrix_getGlMatrix( &instance->trans->world ));
		model_draw( model_fromInstance( instance ) );
	} glPopMatrix();
}

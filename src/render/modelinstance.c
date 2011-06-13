// modelinstance.c

#include "common.h"
#include "modelinstance.h"
//-----------------------
#include "model.h"
#include "transform.h"

modelInstance* modelInstance_create( modelHandle m ) {
	modelInstance* i = malloc( sizeof( modelInstance ));
	i->model = m;
	return i;
}

void modelInstance_draw( modelInstance* instance ) {
	glPushMatrix(); {
		glMultMatrixf( matrix_getGlMatrix( &instance->trans->world ));
		model_draw( model_fromInstance( instance ) );
	} glPopMatrix();
}

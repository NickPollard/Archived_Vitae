// modelinstance.c

#include "common.h"
#include "modelinstance.h"
//-----------------------
#include "model.h"
#include "render/render.h"
#include "transform.h"

IMPLEMENT_POOL( modelInstance )

pool_modelInstance* static_modelInstance_pool = NULL;

void modelInstance_initPool() {
	static_modelInstance_pool = pool_modelInstance_create( 256 );
}

modelInstance* modelInstance_createEmpty( ) {
	modelInstance* i = pool_modelInstance_allocate( static_modelInstance_pool );
	memset( i, 0, sizeof( modelInstance ));
	return i;
}

modelInstance* modelInstance_create( modelHandle m ) {
	modelInstance* i = modelInstance_createEmpty();
	i->model = m;
	return i;
}

void modelInstance_draw( modelInstance* instance ) {
		render_resetModelView();
		matrix_mul( modelview, modelview, instance->trans->world );
		glUniformMatrix4fv( resources.uniforms.modelview, 1, /*transpose*/false, (GLfloat*)modelview );

		model_draw( model_fromInstance( instance ) );
}

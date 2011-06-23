// list.h

typedef struct list_s {
	void* head;
	void* tail;
} list;

#define DEF_LIST(type) \
	typedef struct type##list_s { \
		type* head; \
		struct type##list_s* tail; \
	} type##list;

#define IMPLEMENT_LIST(type)									\
	type##list* type##list_create() {							\
		type##list* list = mem_alloc( sizeof( type##list ));	\
		list->head = NULL;										\
		list->tail = NULL;										\
		return list;											\
	}

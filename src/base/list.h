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

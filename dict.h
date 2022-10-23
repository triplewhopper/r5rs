#ifndef R5RS_DICT_H
#define R5RS_DICT_H
#include "Include/typedefs.h"
typedef struct {
	const char *_type;
	int _ref_cnt;
	void *_obj;
} Object;
u64 object_hash(Object *self){
	if(strcmp(self->_type, "")==0){

	}
}
typedef struct {
	void *buckets;
} Dict;

void dict_setitem(Dict *self, Object *key, Object *value) {

}

#endif //R5RS_DICT_H

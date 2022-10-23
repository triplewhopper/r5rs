#include "../Include/object.h"
#include "../Include/typeobject.h"

ScmTypeObject ScmType_Type = {
		1,
		0,
		0,
		"type",
		0,
		0,
		0,
		0,
		0,
};
ScmTypeObject ScmBaseObject_Type = {
		1,
		0,
		0,
		"object",
		0,
		0,
		0,
		0,
		0,
};
void initialize_global_type_objects(){
	ScmType_Type.ob_base.ob_base.ob_type = &ScmType_Type;
	ScmType_Type.tp_base = &ScmBaseObject_Type;

	ScmBaseObject_Type
}
int main() {

}
#ifndef R5RS_LINKED_LIST_OBJECT_H
#define R5RS_LINKED_LIST_OBJECT_H
#include "typedefs.h"

//struct list_node{
//	Object ob_base;
//	struct list_node *prev;
//	struct list_node *next;
//	Object *data;
//};
//struct ScmLinkedListObject{
//	Object ob_base;
//	size_t size;
//	struct list_node *head;
//	struct list_node *tail;
//};
//ScmTypeObject ScmLinkedListNode_Type = {
//	1, &ScmLinkedListNode_Type, 0,
//	"LinkedListNode",
//	sizeof (struct list_node)
//};
//ScmTypeObject ScmLinkedList_Type = {
//		1, &ScmLinkedList_Type, 0,
//		"LinkedList",
//		sizeof (struct LinkedListObject)
//};
//void ScmLinkedList_init(struct ScmLinkedListObject * const self){
//	self->size=0;
//	self->head=NULL;
//	self->tail=NULL;
//}
//Object *ScmLinkedList_push_back(struct ScmLinkedListObject *const self, Object *item){
//	struct list_node *new_node = (struct list_node*) ScmLinkedListNode_Type.tp_alloc();
//	if(self->size) {
//		assert(self->tail);
//		assert(self->tail->next == NULL);
//		self->tail->next = new_node;
//		new_node->prev = self->tail;
//		new_node->next = NULL;
//	}else{
//		self->head = new_node;
//		self->tail = new_node;
//		Scm_INCREF(ScmObject_CAST(new_node));
//	}
//	self->size++;
//
//}
#endif //R5RS_LINKED_LIST_OBJECT_H

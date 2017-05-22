#ifndef STACK_H
#define STACK_H

#define define_stack(type) \
\
typedef struct Stack_##type Stack_##type;\
struct Stack_##type {\
	type data;\
	Stack_##type* next;\
};\
\
Stack_##type* Stack_new_##type(type dat) {\
	Stack_##type* res = (Stack_##type*)malloc(sizeof(Stack_##type));\
	res->next = NULL;\
	res->data = dat;\
	return res;\
}\
\
void Stack_destroy_##type(Stack_##type** st) {\
	Stack_##type* next;\
	while(*st) {\
		next = (*st)->next;\
		free(*st);\
		*st = next;\
	}\
}\
\
void Stack_push_##type(Stack_##type** st, type dat) {\
	Stack_##type* res = (Stack_##type*)malloc(sizeof(Stack_##type));\
	res->next = *st;\
	res->data = dat;\
	*st = res;\
}\
\
type Stack_pull_##type(Stack_##type** st) {\
	if(!*st) return NULL;\
	type res = (*st)->data;\
	Stack_##type* next = (*st)->next;\
	free(*st);\
	*st = next;\
	return res;\
}

#endif


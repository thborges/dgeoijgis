/*
 * glibwrap.h
 *
 *  Created on: 14/07/2015
 *      Author: 
 */

#ifndef GLIBWRAP_H
#define GLIBWRAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"

/*
 * Types
 */

#define gpointer void*
typedef unsigned short ushort;
#define G_GNUC_WARN_UNUSED_RESULT __attribute__((warn_unused_result))

/* 
 * Allocation functions
 */

//#define ENABLE_MEMTRACE

#ifndef ENABLE_MEMTRACE
#define g_new(ptype, count) (ptype*)malloc(sizeof(ptype) * (count))
#define g_new0(ptype, count) (ptype*)calloc(count, sizeof(ptype))
#define g_free_aux(ptr) free(ptr)
#define g_renew(ptype, pointer, count) (ptype*)realloc(pointer, sizeof(ptype) * (count))
#else
#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION " " __FILE__ ":" S2(__LINE__)
#define g_new(ptype, count) (ptype*)profile_malloc(sizeof(ptype), count, #ptype LOCATION, 0)
#define g_new0(ptype, count) (ptype*)profile_malloc(sizeof(ptype), count, #ptype LOCATION, 1)
#define g_free_aux(pointer) profile_free(pointer)
#define g_renew(ptype, pointer, count) (ptype*)profile_realloc(pointer, sizeof(ptype), count)
#endif


void g_free(void *pointer);
void *g_memdup(const void *mem, unsigned int byte_size);

/*
 * Double Linked List
 */

struct GList {
	struct GList *next;
	struct GList *priour;
	void *data;
};

typedef struct GList GList;

#define g_list_first(lst) (lst)
#define g_list_previous(lst) (lst->priour)
#define g_list_next(lst) (lst->next)
#define g_list_foreach(lst, var) for(GList *var = g_list_first(lst); var != NULL; var = g_list_next(var))

GList *g_list_append(GList *lst, void *data);
GList *g_list_prepend(GList *lst, void *data);
GList *g_list_remove_link(GList *list, GList *link);
GList *g_list_delete_link(GList *list, GList *link);
GList *g_list_find(GList *lst, void *data);
GList *g_list_remove(GList *lst, void *data);
GList *g_list_concat(GList *lst, GList *lst2);

unsigned g_list_length(GList *lst);

void g_list_free(GList *lst);
void g_list_free_full(GList *lst, void (*destroy)(void *data));

#ifdef __cplusplus
}
#endif

#endif


//
// Created by raude on 3/29/2023.
//

#ifndef SHELL_GLIST_H
#define SHELL_GLIST_H

#include <stdlib.h>

typedef struct GList {
    int len;
    unsigned capacity;
    void **array;
} GList;

struct GList *createListG();

void appendG(struct GList *list, void *value);

void *getValueAtIndexG(struct GList *list, int index);

int setValueAtIndexG(struct GList *list, int index, void *value);

int removeAtIndexG(struct GList *list, int index);

int addAtIndexG(struct GList *list, int index, void *value);

void clearG(struct GList *list);

#endif //SHELL_GLIST_H

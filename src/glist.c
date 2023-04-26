#include "glist.h"

struct GList *createListG() {
	struct GList *list = (struct GList *) malloc(sizeof(struct GList));
	list->capacity = 32;
	list->len = 0;
	list->array = (char **) malloc(list->capacity * sizeof(char *));
	return list;
}

void appendG(struct GList *list, char *value) {
	if (list->len == list->capacity) {
		list->capacity *= 2;
		list->array = realloc(list->array, list->capacity * sizeof(char *));
	}
	list->array[list->len++] = value;
}

void *getValueAtIndexG(struct GList *list, int index) {
	if (index >= list->len || index < 0) {
		return NULL;
	}
	return list->array[index];
}

int setValueAtIndexG(struct GList *list, int index, char *value) {
	if (index >= list->len || index < 0) {
		return -1;
	}
	list->array[index] = value;
	return 0;
}

int removeAtIndexG(struct GList *list, int index) {
	if (index >= list->len || index < 0) {
		return -1;
	}
	for (int i = index; i < list->len - 1; ++i) {
		list->array[i] = list->array[i + 1];
	}
	list->len--;
	return 0;
}

int addAtIndexG(struct GList *list, int index, char *value) {
	if (index >= list->len || index < 0) {
		return -1;
	}
	if (list->len == list->capacity) {
		list->capacity *= 2;
		list->array = realloc(list->array, list->capacity * sizeof(int));
	}
	for (int i = list->len + 1; i > index; --i) {
		list->array[i] = list->array[i - 1];
	}
	list->array[index] = value;
	list->len++;
	return 0;
}
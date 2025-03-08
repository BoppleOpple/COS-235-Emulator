#include "list.h"
#include <stdio.h>
#include <stdlib.h>

LIST listCreate() {
	// return an empty list
	LIST list = { malloc(sizeof(void*) * DEFAULT_LIST_SIZE), DEFAULT_LIST_SIZE, 0 };
	return list;
}

void listAppendItem(LIST *list, void *item) {
	list->size++;
	
	// if the current item is oob, add more items
	if (list->size >= list->maxSize) {
		// double the size of the array
		list->maxSize <<= 1;
		// and safely reallocate its space
		void **newArray = realloc(list->array, list->maxSize*sizeof(void*));
		if (!newArray) {
			free(list->array);
			printf("could not allocate memory somehow");
			exit(1);
			return;
		}
		
		list->array = newArray;
	}

	// add item to array
	*(list->array + list->size - 1) = item;
}

void *listPop(LIST *list, int i) {
	// if the index is invalid, panic
	if (i < 0 || i >= list->size) {
		printf("index %i out of range in list with %i elements.\n", i, list->size);
		return NULL;
	}
	// declare the returned item
	void *item = NULL;

	// sift that item to the end of the list
	// this could be faster with a circular array or by using a set instead of a list,
	// but i really dont mind this temporarily
	for (int j = i + 1; j < list->size; j++) {
		void *temp = *(list->array + j);
		*(list->array + j) = *(list->array + j - 1);
		*(list->array + j - 1) = temp;
	}

	// return item and set the reference to it in the list to NULL
	item = *(list->array + --list->size);
	*(list->array + list->size) = NULL;
	return item;
}

int listIncludesItem(LIST *list, const void *room) {
	// checks every element in the list against the provided element, and return true if a match is found
	for (int i = 0; i < list->size; i++)
		if (*(list->array + i) == room)
			return 1;
	return 0;
}

void *listGetElement(LIST *list, int i) {
	return *(list->array + i);
}

void listClear(LIST *list) {
	for (int i = 0; i < list->size; i++)
		if (listGetElement(list, i))
			free(listGetElement(list, i));
	// free the array
	free(list->array);

	// and update all vars that might make accessing it possible
	list->array = NULL;
	list->maxSize = 0;
	list->size = 0;
}

void listPrint(LIST *list, const char *format) {
	// print each element according to some specified format
	printf("[ ");

	for (int i = 0; i < list->size; i++) {
		printf(format, listGetElement(list, i));
		printf(" ");
	}

	printf("]\n");
}

void listMapFunction(LIST *list, void (fn)(void *)) {
	for (int i = 0; i < list->size; i++)
		fn(listGetElement(list, i));
}
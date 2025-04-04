#pragma once

typedef struct {
	void **array;
	int maxSize;
	int size;
} LIST;

#define DEFAULT_LIST_SIZE 4

/*
 * creates and returns an empty array list of pointers
**/
LIST listCreate();

/*
 * takes a void pointer and adds it to the list by reference
**/
void listAppendItem(LIST *list, void *item);

/*
 * sifts an item to the end of the array and decrements the size, then returns the item
**/
void *listPop(LIST *list, int i);

/*
 * takes a LIST pointer and checks if it contains another pointer (unused)
**/
int listIncludesItem(LIST *list, const void *item);

/*
 * returns the element at index i, useful for if I want to change how addressing works later
**/
void *listGetElement(LIST *list, int i);

/*
 * frees the contents of the list
**/
void listClear(LIST *list);

/*
 * prints all the contents of the list
**/
void listPrint(LIST *list, const char *format);

/**
 * @brief Applies a function across each element of a list
 * 
 * function must take a pointer and return void.
 * 
 * @param list The LIST* to apply `fn` to
 * @param fn The function to be called on each element of `list`
 */
void listMapFunction(LIST *list, void (fn)(void *));
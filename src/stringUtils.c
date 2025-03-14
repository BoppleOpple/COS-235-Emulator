#include "stringUtils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *slice(const char *s, int start, int end) {
	// standard splitting function
	char *substring = malloc(sizeof(char) * (end - start + 1));
	
	for (int i = start; i < end; i++)
		*(substring + i - start) = *(s + i);

	*(substring + end - start) = 0;

	return substring;
}

// splits a string everywhere a function returns truthy
LIST splitAtFunction(const char *s, int (fn)(const char, void *args), void *args) {
	// init list
	LIST sliced = listCreate();

	// keep track of the beginning of the substring
	int lastFalse = -1;
	for (int i = 0; i < strlen(s); i++) {
		// add a substring to the list if true
		if (fn(*(s + i), args)) {
			listAppendItem(&sliced, slice(s, lastFalse + 1, i));

			// reset the beginning
			lastFalse = i;
		}
	}

	// append the last bit from the last false location to the end
	if (lastFalse != strlen(s) - 1)
		listAppendItem(&sliced, slice(s, lastFalse + 1, strlen(s)));

	return sliced;
}

// helper function that checks if two chars are equal
int charEquality(const char a, void *b) {
	return a == *(char *) b;
}

LIST split(const char *s, char character) {
	// call splitAtFunction with the helper
	return splitAtFunction(s, &charEquality, &character);
}

// helper function that checks if a character is a space
int spaceComparison(const char a, void *_b) {
	return isspace(a);
}

LIST splitSpaces(const char *s) {
	// call splitAtFunction with the helper
	LIST sliced = splitAtFunction(s, &spaceComparison, NULL);

	// remove empties
	for (int i = 0; i < sliced.size; i++) {
		if (*(char *) listGetElement(&sliced, i) == 0) {
			free(listPop(&sliced, i));
		}
	}

	return sliced;
}

char *trim(char *s) {
	char *start = s;
	start = s;

	while(isspace(*start)) start++;

	char *end = start + strlen(start);

	if (end != start) {

		while(isspace(*(end - 1))) end--;

		char *copy = malloc(sizeof(char) * ((end + 1) - start));

		memcpy(copy, start, sizeof(char) * (end - start));

		*(copy + (end - start)) = 0;

		return copy;
	} else {
		char *copy = malloc(sizeof(char));
		*copy = 0;
		return copy;
	}
}

char *upper(const char *s) {
	int len = strlen(s) + 1;
	char *str = malloc(sizeof(char) * len);

	for (int i = 0; i < len; i++)
		str[i] = (s[i] >= 'a' && s[i] <= 'z') ? s[i] + 'A' - 'a' : s[i];

	return str;
}
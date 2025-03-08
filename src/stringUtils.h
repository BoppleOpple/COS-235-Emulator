#pragma once

#include "list.h"

/**
 * @brief creates a new string that contains characters from start to end-1, inclusive
 *
 * @param s char* string to slice
 * @param start int first index to include
 * @param end int first index to not be included
 * @return char* substring
 */
char *slice(const char *s, int start, int end);

/**
 * @brief generates a list of substrings in between each instance of a specified character in a string
 *
 * @param s char* string to split
 * @param character char character to split by
 * @return LIST a list of all of the substrings between the specified characters
 */
LIST split(const char *s, char character);

/**
 * @brief generates a list of substrings in between each whitespace character in a string
 *
 * @param s char* string to split
 * @return LIST a list of all of the substrings between whitespace
 */
LIST splitSpaces(const char *s);

char *upper(const char *s);
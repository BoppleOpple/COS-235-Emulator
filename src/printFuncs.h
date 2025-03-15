#pragma once

/**
 * @brief helper function for printing a list of machine code instructions
 * 
 * @param instruction `int *`
 */
void printInstruction(void *instruction);

/**
 * @brief helper function for printing the value stored in an integer *POINTER*
 * 
 * @param i `int *`
 */
void printIntPointer(void *i);

/**
 * @brief helper function for printing the name field of a PROGRAM*
 * 
 * @param program `PROGRAM *`
 */
void printProgramName(void *program);
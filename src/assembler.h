#pragma once
#include "list.h"

#define NUM_OPCODES 10
#define NUM_FIELD_TYPES 3
#define ASSEMBLER_BUFFER_SIZE 1024
#define MAX_FIELDS 4

typedef enum : unsigned int {
	AND,
	OR,
	XOR,
	ADD,
	ADDI,
	SW,
	LW,
	BEQ,
	BLT,
	LABEL = 0b01111
} OPCODE;

typedef enum : unsigned int {
	INSTRUCTION,
	REGISTER,
	IMMEDIATE
} FIELD;

/**
 * @brief a struct to hold each instruction and its decoded form, for searches
 *
 */
typedef struct {
	const OPCODE encoded;
	const char *decoded;
	const int fieldCount;
	const FIELD fields[MAX_FIELDS];
} OPCODE_DATA;

typedef struct {
	unsigned int value;
	int status;
} MACHINE_CODE;

/**
 * @brief takes a c-string representing a line of assembly and returns its
 * machine code or ERROR if it it malformed
 *
 * @param assembly the line of assembly to assemble
 * @return unsigned int the resulting machine code or ERROR
 */
MACHINE_CODE assembleLine(const char *assembly);

/**
 * @brief takes a filepath and returns a list of assembled machine code
 * instructions (or NULL on error)
 *
 * @param path the path of the file to assemble
 * @return LIST* the final list, or NULL
 */
LIST *assembleFile(const char *path);

/**
 * @brief takes an integer and returns its binary representation
 * 
 * @param n 
 */
void printBinary(unsigned int n);

/**
 * @brief takes an integer and returns its binary representation, with
 * separations representing fields of the machine code.
 *
 * @param n 
 */
void printMachineCode(unsigned int n);
#pragma once
#include "list.h"

#define NUM_OPCODES 10
#define ASSEMBLER_BUFFER_SIZE 1024
#define NUM_ARGUMENTS 4

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
	LABEL = 0b01111,
	ERROR = 0b11111
} OPCODE;

typedef struct {
	const OPCODE encoded;
	const char *decoded;
} OpCodeKVPair;

/**
 * @brief takes a c-string representing a line of assembly and returns its machine code
 * 
 * @param assembly 
 * @return unsigned int 
 */
unsigned int assembleLine(const char *assembly);

LIST *assembleFile(const char *path);

void printBinary(unsigned int n);
void printMachineCode(unsigned int n);
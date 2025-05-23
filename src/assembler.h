#pragma once
#include "list.h"

#define NUM_OPCODES 11
#define NUM_FIELD_TYPES 4
#define ASSEMBLER_BUFFER_SIZE 1024
#define MAX_FIELDS 4

typedef enum : int {
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
	EXIT = -1
} OPCODE;

typedef enum : unsigned int {
	INSTRUCTION_FIELD,
	REGISTER_FIELD,
	IMMEDIATE_FIELD,
	LABEL_FIELD
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
	int id;
	char *name;
	unsigned int address;
	LIST references;
} LABEL_DATA;

typedef struct {
	unsigned int value;
	int status;
} MACHINE_CODE;

extern const int FIELD_SIZES[NUM_FIELD_TYPES];

extern const OPCODE_DATA OPCODES[NUM_OPCODES];

/**
 * @brief takes a c-string representing a line of assembly and returns its
 * machine code or ERROR if it it malformed
 *
 * @param assembly the line of assembly to assemble
 * @param labels the labels to fetch ids from
 * @param address the address of this line of assembly
 * @return unsigned int the resulting machine code or ERROR
 */
MACHINE_CODE assembleLine(const char *assembly, LIST *labels, int address);

/**
 * @brief takes a filepath and returns a list of assembled machine code
 * instructions (or NULL on error)
 *
 * @param path the path of the file to assemble
 * @param startAddress the location of the program in memory
 * @return LIST* the final list, or NULL
 */
LIST *assembleFile(const char *path, int startAddress);

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

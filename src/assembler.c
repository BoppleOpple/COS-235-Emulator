#include "assembler.h"
#include "list.h"
#include "stringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int FIELD_SIZES[NUM_FIELD_TYPES] = {
	5,
	5,
	17
};

// each opcode format as specified by the assignment
const OPCODE_DATA OPCODES[NUM_OPCODES] = {
	{AND,   "AND"  , 4, {INSTRUCTION, REGISTER, REGISTER, REGISTER}},
	{OR,    "OR"   , 4, {INSTRUCTION, REGISTER, REGISTER, REGISTER}},
	{XOR,   "XOR"  , 4, {INSTRUCTION, REGISTER, REGISTER, REGISTER}},
	{ADD,   "ADD"  , 4, {INSTRUCTION, REGISTER, REGISTER, REGISTER}},
	{ADDI,  "ADDI" , 4, {INSTRUCTION, REGISTER, REGISTER, IMMEDIATE}},
	{SW,    "SW"   , 4, {INSTRUCTION, REGISTER, REGISTER, IMMEDIATE}},
	{LW,    "LW"   , 4, {INSTRUCTION, REGISTER, REGISTER, IMMEDIATE}},
	{BEQ,   "BEQ"  , 4, {INSTRUCTION, REGISTER, REGISTER, IMMEDIATE}},
	{BLT,   "BLT"  , 4, {INSTRUCTION, REGISTER, REGISTER, IMMEDIATE}},
	{LABEL, "LABEL", 0, {}}
};

MACHINE_CODE assembleLine(const char *assembly) {

	// TODO
	// CONVERT THIS TO A RETURNED STRUCT WITH ERROR INFO
	MACHINE_CODE result = {0, 0};

	// normalize input
	char *trimmedAssembly = trim(assembly);
	LIST splitLine = splitSpaces(trimmedAssembly);
	char *operation = upper(listGetElement(&splitLine, 0));

	OPCODE_DATA *currentInstructionFormat = NULL;

	// find the opcode in the list
	for (int i = 0; i < NUM_OPCODES; i++) {
		if (strcmp(operation, OPCODES[i].decoded) == 0) {
			currentInstructionFormat = (OPCODE_DATA*) OPCODES + i;
			break;
		}
	}
	
	
	if (*trimmedAssembly == ':') { // if a label is denoted
		// check if it has spaces
		if (splitLine.size == 1) result.value = LABEL;
		else { // error out if it does
			printf("error assembling line! bad label \"%s\"\n", trimmedAssembly + 1);
			result.status = 1;
		}
	} else if (currentInstructionFormat == NULL) { // otherwise, if opcode is bad...
		printf("error assembling line! bad isntruction %s\n", (char *) listGetElement(&splitLine, 0));
		result.status = 1;
	} else if (splitLine.size != currentInstructionFormat->fieldCount) { // otherwise, if args are bad...
		printf("error assembling line! wrong number of arguments:\nExpected: %i\nRecieved: %i\n", currentInstructionFormat->fieldCount, splitLine.size);
		result.status = 1;
	} else { // otherwise, assemble the rest of the line and return it

		for (int i = currentInstructionFormat->fieldCount - 1; i >= 0; i--) {
			const FIELD *currentField = currentInstructionFormat->fields + i;
			const int *fieldSize = FIELD_SIZES + *currentField;
			const char *currentArgument = (char*) listGetElement(&splitLine, i);

			unsigned int value = 0;

			switch (*currentField) {
				case INSTRUCTION:
					value = currentInstructionFormat->encoded;
					break;
				
				case REGISTER:
					if (*currentArgument != 'x') {
						printf("error assembling line! Registers should begin with 'x' (recieved \"%s\")\n", currentArgument);
						result.status = 1;
					} else {
						value = atoi(currentArgument + 1);
					}
					break;
				
				case IMMEDIATE:
					if (*currentArgument == 'x') {
						printf("error assembling line! Immediates should not begin with 'x' (recieved \"%s\")\n", currentArgument);
						result.status = 1;
					} else {
						value = atoi(currentArgument);
					}
					break;
			}

			result.value <<= *fieldSize;
			result.value += (value << *fieldSize) >> *fieldSize;
		}
	}

	// clean up
	listClear(&splitLine);
	free(operation);
	free(trimmedAssembly);

	// return result or ERROR
	return result;
}

LIST *assembleFile(const char *path) {
	// prepare buffer for fgets
	char buffer[ASSEMBLER_BUFFER_SIZE];

	// attempt to open the file, and error out if it doesn't exist
	FILE *inFile = fopen(path, "r");
	if (!inFile) {
		printf("error accessing file at %s\n", path);
		return NULL;
	}

	// prepare the list of machine code instructions
	LIST *machineCodeList = malloc(sizeof(LIST));
	*machineCodeList = listCreate();
	
	// store the line count for error checking
	int line = 0;
	unsigned int *currentInstruction = NULL;
	while (fgets(buffer, sizeof(buffer), inFile)) {
		line++;

		// ignore empty lines
		if (*buffer == '\n') continue;

		// allocate memory for the next instruction
		currentInstruction = malloc(sizeof(unsigned int));
		MACHINE_CODE assemblyResult = assembleLine(buffer);

		// if an error was thrown, error out and clean up
		if (assemblyResult.status) {
			printf("Error assembling %s at line %i!\n", path, line);
			fclose(inFile);
			listClear(machineCodeList);
			free(machineCodeList);
			return NULL;
		}

		*currentInstruction = assemblyResult.value;
		
		// otherwise, add the code to the list
		listAppendItem(machineCodeList, currentInstruction);
		currentInstruction = NULL;
	}

	//clean up
	fclose(inFile);
	
	// return the list if no errors occured
	return machineCodeList;
}

void printBinary(unsigned int n) {
	// iterate though each bit of an int
	for (int i = 31; i >= 0; i--) {
		// the current bit is 1 if its place value fits in n
		int bitIsOne = (1 << i <= n);

		// print the bit
		printf("%i", bitIsOne);

		// decrement n for future place values
		n -= bitIsOne * 1 << i;
	}

	// end the line
	printf("\n");
}

void printMachineCode(unsigned int n) {
	// iterate though each bit of an int
	for (int i = 31; i >= 0; i--) {
		// the current bit is 1 if its place value fits in n
		int bitIsOne = (1 << i <= n);

		// print space if it is between two ASM arguments
		if (i == 4 || i == 9 || i == 14) printf(" ");

		// print the bit
		printf("%i", bitIsOne);
		
		// decrement n for future place values
		n -= bitIsOne * (1 << i);
	}

	// end the line
	printf("\n");
}
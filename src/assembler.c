#include "assembler.h"
#include "list.h"
#include "stringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// each opcode format as specified by the assignment
const OpCodeKVPair OPCODES[NUM_OPCODES] = {
	{AND,   "AND"  },
	{OR,    "OR"   },
	{XOR,   "XOR"  },
	{ADD,   "ADD"  },
	{ADDI,  "ADDI" },
	{SW,    "SW"   },
	{LW,    "LW"   },
	{BEQ,   "BEQ"  },
	{BLT,   "BLT"  },
	{LABEL, "LABEL"}
};

unsigned int assembleLine(const char *assembly) {
	// default to -1, which is impossible with ERROR = 0b11111
	int result = -1;

	// normalize input
	char *trimmedAssembly = trim(assembly);
	LIST splitLine = splitSpaces(trimmedAssembly);
	char *operation = upper(listGetElement(&splitLine, 0));

	// find the opcode in the list
	// if it is not found, result will remain as -1
	for (int i = 0; i < NUM_OPCODES; i++) {
		if (strcmp(operation, OPCODES[i].decoded) == 0) {
			result = OPCODES[i].encoded;
			break;
		}
	}
	
	// if a label is denoted
	if (*trimmedAssembly == ':') {
		// check if it has spaces
		if (splitLine.size == 1) result = LABEL;
		else { // error out if it does
			printf("error assembling line! bad label \"%s\"\n", trimmedAssembly + 1);
			result = ERROR;
		}
	} else if (splitLine.size != NUM_ARGUMENTS) { // otherwise, if args are bad...
		printf("error assembling line! wrong number of arguments:\nExpected: %i\nRecieved: %i\n", NUM_ARGUMENTS, splitLine.size);
		result = ERROR;
	} else if (result == -1) { // otherwise, if opcode is bad...
		printf("error assembling line! bad isntruction %s\n", (char *) listGetElement(&splitLine, 0));
		result = ERROR;
	} else { // otherwise, assemble the rest of the line and return it

		// rd goes in the next 5 bits
		result += (atoi(listGetElement(&splitLine, 1)) & 0b11111) << 5;

		// rs1 goes in the next 5 bits
		result += (atoi(listGetElement(&splitLine, 2)) & 0b11111) << 10;

		// rs2/imm/label goes in remaining bits
		result += (atoi(listGetElement(&splitLine, 3)) & 0b11111111111111111) << 15;
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
		*currentInstruction = assembleLine(buffer);

		// if an error was thrown, error out and clean up
		if ((*currentInstruction & 0b11111) == ERROR) {
			printf("Error assembling %s at line %i!\n", path, line);
			listClear(machineCodeList);
			free(machineCodeList);
			return NULL;
		}
		
		// otherwise, add the code to the list
		listAppendItem(machineCodeList, currentInstruction);
		currentInstruction = NULL;
	}

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
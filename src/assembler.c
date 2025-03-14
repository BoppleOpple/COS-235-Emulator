#include "assembler.h"
#include "list.h"
#include "stringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const OpCodeKVPair OPCODES[NUM_OPCODES] = {
	{0b00000, "AND"},
	{0b00001, "OR"},
	{0b00010, "XOR"},
	{0b00011, "ADD"},
	{0b00100, "ADDI"},
	{0b00101, "SW"},
	{0b00110, "LW"},
	{0b00111, "BEQ"},
	{0b01000, "BLT"},
	{0b01111, "LABEL"}
};

unsigned int assembleLine(const char *assembly) {
	int result = 0;
	int fieldCount;
	LIST splitLine = splitSpaces(assembly);

	char *operation = upper(listGetElement(&splitLine, 0));

	for (int i = 0; i < NUM_OPCODES; i++) {
		if (strcmp(operation, OPCODES[i].decoded) == 0) {
			result += OPCODES[i].encoded;
		}
	}

	result += (atoi(listGetElement(&splitLine, 1)) & 0b11111) << 5;
	result += (atoi(listGetElement(&splitLine, 2)) & 0b11111) << 10;
	result += (atoi(listGetElement(&splitLine, 3)) & 0b11111111111111111) << 15;

	listClear(&splitLine);
	free(operation);

	return result;
}

LIST *assembleFile(const char *path) {
	char buffer[ASSEMBLER_BUFFER_SIZE];
	FILE *inFile = fopen(path, "r");

	if (!inFile) {
		printf("error reading file at %s\n", path);
		return NULL;
	}

	LIST *machineCodeList = malloc(sizeof(LIST));
	*machineCodeList = listCreate();
	

	unsigned int *currentInstruction = NULL;
	while (fgets(buffer, sizeof(buffer), inFile)) {
		currentInstruction = malloc(sizeof(unsigned int));

		*currentInstruction = assembleLine(buffer);
		
		listAppendItem(machineCodeList, currentInstruction);
		currentInstruction = NULL;
	}

	return machineCodeList;
}

void printBinary(unsigned int n) {
	for (int i = 31; i >= 0; i--) {
		int bitIsOne = (1 << i <= n);
		printf("%i", bitIsOne);
		n -= bitIsOne * 1 << i;
	}

	printf("\n");
}

void printMachineCode(unsigned int n) {
	for (int i = 31; i >= 0; i--) {
		int bitIsOne = (1 << i <= n);
		if (i == 4 || i == 9 || i == 14) printf(" ");
		printf("%i", bitIsOne);
		n -= bitIsOne * (1 << i);
	}

	printf("\n");
}
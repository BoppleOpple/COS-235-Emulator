#include "assembler.h"
#include "list.h"
#include "stringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	int result = -1;

	char *trimmedAssembly = trim(assembly);

	LIST splitLine = splitSpaces(trimmedAssembly);

	char *operation = upper(listGetElement(&splitLine, 0));

	for (int i = 0; i < NUM_OPCODES; i++) {
		if (strcmp(operation, OPCODES[i].decoded) == 0) {
			result = OPCODES[i].encoded;
			break;
		}
	}
	
	if (*trimmedAssembly == ':') {
		if (splitLine.size == 1) result = LABEL;
		else {
			printf("error assembling line! bad label \"%s\"\n", trimmedAssembly + 1);
			result = ERROR;
		}
	} else if (splitLine.size != NUM_ARGUMENTS) {
		printf("error assembling line! wrong number of arguments:\nExpected: %i\nRecieved: %i\n", NUM_ARGUMENTS, splitLine.size);
		result = ERROR;
	} else if (result == -1) {
		printf("error assembling line! bad isntruction %s\n", (char *) listGetElement(&splitLine, 0));
		result = ERROR;
	} else {

		result += (atoi(listGetElement(&splitLine, 1)) & 0b11111) << 5;
		result += (atoi(listGetElement(&splitLine, 2)) & 0b11111) << 10;
		result += (atoi(listGetElement(&splitLine, 3)) & 0b11111111111111111) << 15;
	}

	listClear(&splitLine);
	free(operation);
	free(trimmedAssembly);
	
	return result;
}

LIST *assembleFile(const char *path) {
	char buffer[ASSEMBLER_BUFFER_SIZE];
	FILE *inFile = fopen(path, "r");

	if (!inFile) {
		printf("error accessing file at %s\n", path);
		return NULL;
	}

	LIST *machineCodeList = malloc(sizeof(LIST));
	*machineCodeList = listCreate();
	
	int line = 0;
	unsigned int *currentInstruction = NULL;
	while (fgets(buffer, sizeof(buffer), inFile)) {
		line++;

		if (*buffer == '\n') continue;

		currentInstruction = malloc(sizeof(unsigned int));

		*currentInstruction = assembleLine(buffer);

		if ((*currentInstruction & 0b11111) == ERROR) {
			printf("Error assembling %s at line %i!\n", path, line);
			listClear(machineCodeList);
			free(machineCodeList);
			return NULL;
		}
		
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
#include "assembler.h"
#include "list.h"
#include "printFuncs.h"
#include "stringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int FIELD_SIZES[NUM_FIELD_TYPES] = {
	5,
	5,
	17,
	17
};

// each opcode format as specified by the assignment
const OPCODE_DATA OPCODES[NUM_OPCODES] = {
	{AND,   "AND"  , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, REGISTER_FIELD}},
	{OR,    "OR"   , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, REGISTER_FIELD}},
	{XOR,   "XOR"  , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, REGISTER_FIELD}},
	{ADD,   "ADD"  , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, REGISTER_FIELD}},
	{ADDI,  "ADDI" , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, IMMEDIATE_FIELD}},
	{SW,    "SW"   , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, IMMEDIATE_FIELD}},
	{LW,    "LW"   , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, IMMEDIATE_FIELD}},
	{BEQ,   "BEQ"  , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, LABEL_FIELD}},
	{BLT,   "BLT"  , 4, {INSTRUCTION_FIELD, REGISTER_FIELD, REGISTER_FIELD, LABEL_FIELD}},
	{LABEL, "LABEL", 0, {}},
	{EXIT, "EXIT", 0, {}}
};

MACHINE_CODE assembleLine(const char *assembly, LIST *labels, int address) {
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
		if (splitLine.size == 1) {
			for (int i = 0; i < labels->size; i++) {
				LABEL_DATA *label = listGetElement(labels, i);
				if (strcmp(label->name, trimmedAssembly) == 0) {
					label->address = address;
					break;
				}
			}
			
			result.value = LABEL;
		} else { // error out if it does
			printf("error assembling line! bad label \"%s\"\n", trimmedAssembly);
			result.status = 1;
		}
	} else if (currentInstructionFormat == NULL) { // otherwise, if opcode is bad...
		printf("error assembling line! bad isntruction %s\n", (char *) listGetElement(&splitLine, 0));
		result.status = 1;
	} else if (splitLine.size != currentInstructionFormat->fieldCount) { // otherwise, if args are bad...
		printf("error assembling line! wrong number of arguments:\nExpected: %i\nRecieved: %i\n", currentInstructionFormat->fieldCount, splitLine.size);
		result.status = 1;
	} else { // otherwise, assemble the rest of the line and return it

		// iterate through the fields present in the instruction start at the most
		// significant and move right, to allow everything to be operated on in the
		// least significant bits
		for (int i = currentInstructionFormat->fieldCount - 1; i >= 0; i--) {
			const FIELD *currentField = currentInstructionFormat->fields + i;
			const int *fieldSize = FIELD_SIZES + *currentField;
			const char *currentArgument = (char*) listGetElement(&splitLine, i);

			// variables for the switch statement
			unsigned int value = 0;
			LABEL_DATA *label = NULL;

			switch (*currentField) {
				case INSTRUCTION_FIELD:
					// for the instruction field, the value is already parsed
					value = currentInstructionFormat->encoded;
					break;
				
				case REGISTER_FIELD:
					// registers must begin with 'x'
					if (*currentArgument != 'x') {
						printf("error assembling line! Registers should begin with 'x' (recieved \"%s\")\n", currentArgument);
						result.status = 1;
					} else {
						value = atoi(currentArgument + 1);
					}
					break;
				
				case IMMEDIATE_FIELD:
					// immediates must NOT begin with 'x'
					if (*currentArgument == 'x') {
						printf("error assembling line! Immediates should not begin with 'x' (recieved \"%s\")\n", currentArgument);
						result.status = 1;
					} else {
						value = atoi(currentArgument);
					}
					break;
				
				case LABEL_FIELD:
					// labels must exist in the passed label list
					for (int i = 0; i < labels->size; i++) {
						if (strcmp(((LABEL_DATA *) listGetElement(labels, i))->name, currentArgument) == 0) {
							label = listGetElement(labels, i);

							// fill the field with the label id, to be processed later
							value = label->id;

							int *ref = malloc(sizeof(int));
							*ref = address;

							listAppendItem(&label->references, ref);
							break;
						}
					}
					if (!label) {
						printf("error assembling line! could not find label \"%s\"\n", currentArgument);
						result.status = 1;
					}
					break;
			}
			// move the old data back to make room for the new field
			result.value <<= *fieldSize;

			// mask the field to keep it in bounds and add it to result
			unsigned int mask = (-1 << *fieldSize) ^ -1;
			result.value += value & mask;
		}
	}

	// clean up
	listClear(&splitLine);
	free(operation);
	free(trimmedAssembly);

	// return result or ERROR
	return result;
}
LIST *indexLabels(const char *path) {
	LIST *labels = malloc(sizeof(LIST));
	*labels = listCreate();

	// prepare buffer for fgets
	char buffer[ASSEMBLER_BUFFER_SIZE];

	// attempt to open the file, and error out if it doesn't exist
	FILE *inFile = fopen(path, "r");
	if (!inFile) {
		printf("error accessing file at %s\n", path);
		return NULL;
	}
	int id = 0;
	while (fgets(buffer, sizeof(buffer), inFile)) {
		if (*buffer != ':') continue;

		LABEL_DATA *label = malloc(sizeof(LABEL_DATA));
		label->id = id++;
		label->name = trim(buffer);
		label->address = 0;
		label->references = listCreate();

		listAppendItem(labels, label);
	}

	fclose(inFile);

	return labels;
}

void freeLabelData(void *label) {
	free(((LABEL_DATA*) label)->name);
	listClear(&((LABEL_DATA*) label)->references);
	((LABEL_DATA*) label)->name = NULL;
}

void updateLabels(LIST *labels, LIST *machineCodeList, int startAddress) {
	// for each reference of each label...
	for (int i = 0; i < labels->size; i++) {
		LABEL_DATA *label = listGetElement(labels, i);

		for (int j = 0; j < label->references.size; j++) {
			// fetch the specified instruction
			int index = *(int*) listGetElement(&label->references, j) - startAddress;
			unsigned int *updatedInstruction = (unsigned int*) listGetElement(machineCodeList, index);

			// find the correct opcode
			for (int k = 0; k < NUM_OPCODES; k++) {
				if (OPCODES[k].encoded == (*updatedInstruction & 0b11111)) {
					// store the lowest bit of the current field, so only that fields bits may be modified later
					int fieldLowestBit = 0;

					// find the corresponding label field
					for (int l = 0; l < OPCODES[k].fieldCount; l++) {
						FIELD field = OPCODES[k].fields[l];
						int fieldSize = FIELD_SIZES[field];

						// consider LABEL_FIELD describes the 3 highest-order bits of an 8-bit number
						if (field == LABEL_FIELD) {
							// then, mask is 00000111
							unsigned int mask = (-1 << fieldSize) ^ -1;

							// and the value of the current field can be found by shifting the data to align the lowest-order bit with the 0 bit:
							// data:  1011 1001
							// field: 1110 0000
							// mask:  0000 0111
							//
							//   data >> 5 | 0000 0101 (1100 1)
							// & mask      | 0000 0111
							// -----------------------
							//   id        | 0000 0101
							int id = (*updatedInstruction >> fieldLowestBit) & mask;

							// if the id matches, the address should be inserted
							// this can be done by cutting the fiels out of the instruction and summing that with the new data shifted to the correct bit:
							// data:  1011 1001
							// field: 1110 0000
							//
							//   other data |    1 1001
							// + new data   | 011      
							// ------------------------
							//   new instr. | 0111 1001
							if (id == label->id)
								*updatedInstruction = (*updatedInstruction & ((mask << fieldLowestBit) ^ -1)) + ((label->address & mask) << fieldLowestBit);
						}

						fieldLowestBit += fieldSize;
					}
					break;
				}
			} // end of opcode loop, can be broken

		} // end of reference loop

	} // end of label loop
}

LIST *assembleFile(const char *path, int startAddress) {
	// prepare buffer for fgets
	char buffer[ASSEMBLER_BUFFER_SIZE];

	// attempt to open the file, and error out if it doesn't exist
	FILE *inFile = fopen(path, "r");
	if (!inFile) {
		printf("error accessing file at %s\n", path);
		return NULL;
	}

	LIST *labels = indexLabels(path);

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
		MACHINE_CODE assemblyResult = assembleLine(buffer, labels, startAddress + machineCodeList->size);

		// if an error was thrown, error out and clean up
		if (assemblyResult.status) {
			printf("Error assembling %s at line %i!\n", path, line);
			fclose(inFile);

			// free label list
			listMapFunction(labels, *freeLabelData);
			listClear(labels);
			free(labels);

			// free machine code list
			listClear(machineCodeList);
			free(machineCodeList);
			return NULL;
		}

		*currentInstruction = assemblyResult.value;
		
		// otherwise, add the code to the list
		listAppendItem(machineCodeList, currentInstruction);
		currentInstruction = NULL;
	}

	// printf("LABELS:\n");
	// listMapFunction(labels, *printLabelData);

	// replace the label ids with the correct addresses
	updateLabels(labels, machineCodeList, startAddress);

	// finally, add an exit instruction
	currentInstruction = malloc(sizeof(unsigned int));
	*currentInstruction = EXIT;
	listAppendItem(machineCodeList, currentInstruction);
	currentInstruction = NULL;


	// clean up
	fclose(inFile);
	listMapFunction(labels, *freeLabelData);
	listClear(labels);
	free(labels);
	
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
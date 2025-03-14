#include "assembler.h"
#include "list.h"
#include "stringUtils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_SIZE_BYTES 4
#define MEMORY_SIZE_KB 16
#define MEMORY_SIZE_BYTES ( 1 << 10 ) * MEMORY_SIZE_KB
#define MEMORY_SIZE_WORDS ( MEMORY_SIZE_BYTES / WORD_SIZE_BYTES )

#define REGISTER_COUNT 32
#define COMMAND_COUNT 6

typedef struct {
	int* entryPoint;
	int size;
} PROGRAM;

typedef enum : char {
	NONE = -1,
	load,
	run,
	purgep,
	purged,
	save,
	quit
} OPTION_ID;

typedef struct {
	const char* command;
	const char* description;
} MENU_OPTION;


const MENU_OPTION commands[COMMAND_COUNT] = {
	{"load", "Load Program"},
	{"run", "Run Program"},
	{"purgep", "Purge All Programs"},
	{"purged", "Purge All Data"},
	{"save", "Save to Disk"},
	{"quit", "Quit"}
};

int memory[MEMORY_SIZE_WORDS];
int registers[REGISTER_COUNT];

LIST programMemory = {0, 0 ,0};

void clearScreen() {
	// just print the ansi
	printf("\033[2J\033[H");
}

void resetMemory() {
	for (int i = 0; i < MEMORY_SIZE_WORDS; i++) memory[i] = 0;
	
	listClear(&programMemory);
	LIST programMemory = listCreate();
}

void resetRegisters() {
	// set all registers to 0
	for (int i = 0; i < REGISTER_COUNT; i++) registers[i] = 0;

	// set the stack pointer to the end of memory
	registers[2] = MEMORY_SIZE_WORDS;
}

void printInstruction(void *instruction) {
	printMachineCode(*(int*) instruction);
}

void printIntPointer(void *i) {
	printf("%i\n", *(int*) i);
}

void addProgram(LIST *program) {
	PROGRAM *programContainer = malloc(sizeof(PROGRAM));
	if (programMemory.size > 0) {
		PROGRAM *lastProgram = (PROGRAM *) listGetElement(&programMemory, programMemory.size-1);

		programContainer->entryPoint = lastProgram->entryPoint + lastProgram->size;
	} else {
		programContainer->entryPoint = memory;
	}
	programContainer->size = program->size;
	listAppendItem(&programMemory, programContainer);

	for (int i = 0; i < programContainer->size; i++) {
		*(programContainer->entryPoint + i) = *(int *) listGetElement(program, i);
	}
}

void dumpMemory(const char *filepath) {
	FILE *file = fopen(filepath, "w");

	for (int i = 0; i < MEMORY_SIZE_WORDS; i++)
		fprintf(file, "%u: %i\n", i, memory[i]);

	fclose(file);
}

int main() {
	resetMemory();

	printf("bytes of memory: %i\n", MEMORY_SIZE_BYTES);
	printf("words of memory: %i\n", MEMORY_SIZE_WORDS);

	////

	OPTION_ID selectedOption;
	char inBuffer[512];
	int loop = 1;

	clearScreen();

	while (loop) {
		selectedOption = -1;

		for (int i = 0; i < COMMAND_COUNT; i++)
			printf("%s: %s\n", commands[i].command, commands[i].description);
		
		fgets(inBuffer, sizeof(inBuffer), stdin);
		
		char *trimmed = inBuffer + strlen(inBuffer) - 1;

		while(isspace(*trimmed)) trimmed--;

		*(trimmed + 1) = 0;

		trimmed = inBuffer;

		while(isspace(*trimmed)) trimmed++;

		for (int i = 0; i < COMMAND_COUNT; i++) {
			if (strcmp(commands[i].command, trimmed) == 0) {
				selectedOption = i;
				break;
			}
		}

		switch (selectedOption) {
			case load:
				printf("loading\n");
				break;

			case run:
				printf("running program\n");
				break;
				
			case purgep:
				printf("purging program\n");
				break;
				
			case purged:
				printf("purging data\n");
				break;
				
			case save:
				printf("saving memory\n");
				break;

			case quit:
				printf("exiting\n");
				loop = 0;
				break;
			
			default:
				printf("this option is not supported\n");
		}
		printf("\npress ENTER to continue\n");

		fgets(inBuffer, sizeof(inBuffer), stdin);
		
		clearScreen();
	}

	////
	listClear(&programMemory);

	return 0;
}
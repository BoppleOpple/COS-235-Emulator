#include "assembler.h"
#include "list.h"
#include "printFuncs.h"
#include "program.h"
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
#define COMMAND_COUNT 7

typedef enum : char {
	NONE = -1,
	load,
	run,
	purgep,
	purged,
	save,
	list,
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
	{"list", "List Loaded Programs"},
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
	programMemory = listCreate();
}

void resetPrograms() {
	PROGRAM *lastProgram = (PROGRAM*) listGetElement(&programMemory, programMemory.size-1);
	for (int i = 0; i < lastProgram->startAddress + lastProgram->size; i++) memory[i] = 0;

	listClear(&programMemory);
	programMemory = listCreate();
}

void resetData() {
	PROGRAM *lastProgram = (PROGRAM*) listGetElement(&programMemory, programMemory.size-1);
	for (int i = lastProgram->startAddress + lastProgram->size; i < MEMORY_SIZE_WORDS; i++) memory[i] = 0;
}

void resetRegisters() {
	// set all registers to 0
	for (int i = 0; i < REGISTER_COUNT; i++) registers[i] = 0;

	// set the stack pointer to the end of memory
	registers[2] = MEMORY_SIZE_WORDS;
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
	char *trimmed = NULL;

	clearScreen();

	while (loop) {
		selectedOption = -1;

		for (int i = 0; i < COMMAND_COUNT; i++)
			printf("%s: %s\n", commands[i].command, commands[i].description);
		
		fgets(inBuffer, sizeof(inBuffer), stdin);
		
		trimmed = trim(inBuffer);

		for (int i = 0; i < COMMAND_COUNT; i++) {
			if (strcmp(commands[i].command, trimmed) == 0) {
				selectedOption = i;
				break;
			}
		}

		free(trimmed);
		trimmed = NULL;

		printf("\n");

		switch (selectedOption) {
			case load:
				printf("Provide the path of the program to load:\n");

				fgets(inBuffer, sizeof(inBuffer), stdin);

				trimmed = trim(inBuffer);

				PROGRAM *newProgram = addProgram(trimmed, memory, &programMemory);

				free(trimmed);
				trimmed = NULL;

				if (!newProgram) {
					printf("program couldn't be loaded into memory\n");
					break;
				}

				printf("Loaded program into memory at location %i\n", ((PROGRAM *) listGetElement(&programMemory, programMemory.size-1))->startAddress);
				break;

			case run:
				printf("Provide the name of the program to run:\n");

				fgets(inBuffer, sizeof(inBuffer), stdin);

				trimmed = trim(inBuffer);

				printf("Attempting to run program \"%s\"\n", trimmed);

				PROGRAM *loadedProgram = NULL;

				for (int i = 0; i < programMemory.size; i++) {
					if (strcmp(trimmed, ((PROGRAM *) listGetElement(&programMemory, i))->name) == 0) {
						loadedProgram = (PROGRAM *) listGetElement(&programMemory, i);
						break;
					}
				}

				if (!loadedProgram) {
					printf("Error loading program! Use the \"list\" command to list loaded programs\n");
					break;
				}

				printf("Program found at location %i\n", loadedProgram->startAddress);
				break;
				
			case purgep:
				printf("Purging programs...\n");
				resetPrograms();
				printf("All program memory cleared!\n");
				break;
				
			case purged:
				printf("Purging data...\n");
				resetData();
				printf("All non-program memory cleared!\n");
				break;
				
			case save:
				printf("saving memory to ./output/disk.txt\n");
				dumpMemory("./output/disk.txt");
				break;

			case list:
				printf("Listing every program name:\n");
				listMapFunction(&programMemory, *printProgramName);
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
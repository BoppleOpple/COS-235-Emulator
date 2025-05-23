#include "assembler.h"
#include "cache.h"
#include "list.h"
#include "printFuncs.h"
#include "program.h"
#include "stringUtils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

// define constants

#define WORD_SIZE_BYTES 4
#define MEMORY_SIZE_KB 16
#define MEMORY_SIZE_BYTES ( 1 << 10 ) * MEMORY_SIZE_KB
#define MEMORY_SIZE_WORDS ( MEMORY_SIZE_BYTES / WORD_SIZE_BYTES )

#define REGISTER_COUNT 32
#define COMMAND_COUNT 7

// define IDs for each menu option, so more can be added easily

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
	OPCODE instruction;
	int arguments[MAX_FIELDS - 1];
} DECODED_INSTRUCTION;

// store the command and its description for each command, in the same index as in the above enum

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

// create arrays for memory and registers

int memory[MEMORY_SIZE_WORDS];
int registers[REGISTER_COUNT];
CACHE *cache;

// create a list for program memory, and ensure there are no garbage values
LIST programMemory = {0, 0 ,0};

/**
 * @brief helper function for clearing the screen
 */
void clearScreen() {
	// just print the ansi
	printf("\033[2J\033[H");
}

/**
 * @brief resets the memory array, programMemory list, and stack pointer
 */
void resetMemory() {
	// set all words to 0
	for (int i = 0; i < MEMORY_SIZE_WORDS; i++) memory[i] = 0;
	
	// clear program data
	listClear(&programMemory);
	programMemory = listCreate();

	if (cache) free(cache);
	cache = createCache();

	// set stack pointer to end of memory
	registers[2] = MEMORY_SIZE_WORDS - 1;
}

/**
 * @brief zeros out the programMemory elements and their associated code
 */
void resetPrograms() {
	// find the index to clear until
	PROGRAM *lastProgram = (PROGRAM*) listGetElement(&programMemory, programMemory.size-1);

	// clear all those addresses
	for (int i = 0; i < lastProgram->startAddress + lastProgram->size; i++) memory[i] = 0;

	// clear the list of associated data
	listClear(&programMemory);
	programMemory = listCreate();
}

/**
 * @brief zeros out all non-program data
 */
void resetData() {
	// find the index to preserve until
	PROGRAM *lastProgram = (PROGRAM*) listGetElement(&programMemory, programMemory.size-1);

	// clear all words past that point
	for (int i = lastProgram->startAddress + lastProgram->size; i < MEMORY_SIZE_WORDS; i++) memory[i] = 0;
}

/**
 * @brief clears the values in all the registers
 */
void resetRegisters() {
	// store the stack pointer
	int sp = registers[2];

	// set all registers to 0
	for (int i = 0; i < REGISTER_COUNT; i++) registers[i] = 0;

	// set the stack pointer to the last value
	registers[2] = sp;
}

/**
 * @brief saves memory prettily to a file
 * 
 * @param filepath the destination of the memory dump
 */
void dumpMemory(const char *filepath) {
	// get the directory path from the filepath
	struct stat st = {0};

	// store the last occurance of '/'
	int filenameIndex = lastIndexInString(filepath, '/') + 1;

	char *directory = slice(filepath, 0, filenameIndex);

	// if the directory does not exist, create it with drwx------
	if (stat(directory, &st) == -1) {
		mkdir(directory, 0700);
	}

	free(directory);
	directory = NULL;

	// open the file if it can be opened
	FILE *file = fopen(filepath, "w");

	if (!file) {
		printf("error dumping memory! file %s couldnt be created or modified\n", filepath);
		return;
	}

	printCacheToFile(cache, file);

	fprintf(file, "\nRAM\n");

	// print each word to a line of the file
	for (int i = 0; i < MEMORY_SIZE_WORDS; i++)
		fprintf(file, "%u: %i\n", i, memory[i]);
	
	// close the file
	fclose(file);
}

void printRegisters() {
	for (int i = 0; i < REGISTER_COUNT; i++) 
		printf("%2i: %i\n", i, registers[i]);
}

DECODED_INSTRUCTION decode(unsigned int instruction) {
	DECODED_INSTRUCTION decoded = {instruction & 0b11111, {}};

	instruction >>= 5;

	OPCODE_DATA *opcode = NULL;

	for (int i = 0; i < NUM_OPCODES; i++) {
		if (decoded.instruction == OPCODES[i].encoded) {
			opcode = (OPCODE_DATA *) OPCODES + i;
			break;
		}
	}

	if (opcode == NULL) {
		decoded.instruction = EXIT;
		return decoded;
	}

	for (int i = 1; i < opcode->fieldCount; i++) {
		const int *fieldSize = FIELD_SIZES + opcode->fields[i];

		// mask of `fieldSize` ones
		unsigned int mask = (-1 << *fieldSize) ^ -1;
		
		decoded.arguments[i - 1] = instruction & mask;
		instruction >>= *fieldSize;

		// if its signed, fill with 1s
		if (opcode->fields[i] == IMMEDIATE_FIELD) {
			decoded.arguments[i - 1] <<= (32 - *fieldSize);
			decoded.arguments[i - 1] >>= (32 - *fieldSize);
		}
	}

	return decoded;
}

int execute(DECODED_INSTRUCTION instruction, unsigned int *pc, int *loadHits, int *loadMisses, int *storeHits, int *storeMisses) {
	switch (instruction.instruction) {
		case AND:
			registers[instruction.arguments[0]] = registers[instruction.arguments[1]] & registers[instruction.arguments[2]];
			break;

		case OR:
			registers[instruction.arguments[0]] = registers[instruction.arguments[1]] | registers[instruction.arguments[2]];
			break;

		case XOR:
			registers[instruction.arguments[0]] = registers[instruction.arguments[1]] ^ registers[instruction.arguments[2]];
			break;
		
		case ADD:
			registers[instruction.arguments[0]] = registers[instruction.arguments[1]] + registers[instruction.arguments[2]];
			break;
		
		case ADDI:
			registers[instruction.arguments[0]] = registers[instruction.arguments[1]] + instruction.arguments[2];
			break;
		
		case SW:
			cacheStoreWord(cache, memory, registers[instruction.arguments[1]] + instruction.arguments[2], storeHits, storeMisses, registers[instruction.arguments[0]]);
			break;
		
		case LW:
			registers[instruction.arguments[0]] = cacheLoadWord(cache, memory, registers[instruction.arguments[1]] + instruction.arguments[2], loadHits, loadMisses);
			break;
		
		case BEQ:
			if (registers[instruction.arguments[0]] == registers[instruction.arguments[1]]) *pc = instruction.arguments[2];
			break;
		
		case BLT:
			if (registers[instruction.arguments[0]] < registers[instruction.arguments[1]]) *pc = instruction.arguments[2];
			break;
		
		case EXIT:
			return 1;
			break;

		default:
			// NOOP
			break;
	}

	return 0;
}

void printProgramStats(int loadHits, int loadMisses, int storeHits, int storeMisses) {
	int loads = loadHits + loadMisses;
	int stores = storeHits + storeMisses;
	int total = loads + stores;

	printf("Total Loads/Stores: %i\n", total);
	printf("Total Hits: %i (%.1f%%)\n", loadHits + storeHits, 100.0 * (float) (loadHits + storeHits) / (float) total);
	printf("Total Misses: %i (%.1f%%)\n\n", loadMisses + storeMisses, 100.0 * (float) (loadMisses + storeMisses) / (float) total);

	printf("Loads: %i\n", loads);
	printf("Hits: %i (%.1f%%)\n", loadHits, 100.0 * (float) loadHits / (float) loads);
	printf("Misses: %i (%.1f%%)\n\n", loadMisses, 100.0 * (float) loadMisses / (float) loads);

	printf("Stores: %i\n", stores);
	printf("Hits: %i (%.1f%%)\n", storeHits, 100.0 * (float) storeHits / (float) stores);
	printf("Misses: %i (%.1f%%)\n\n", storeMisses, 100.0 * (float) storeMisses / (float) stores);
}

int main() {
	// initialize memory
	resetMemory();

	// print some info for debugging
	// printf("bytes of memory: %i\n", MEMORY_SIZE_BYTES);
	// printf("words of memory: %i\n", MEMORY_SIZE_WORDS);

	/*-- main loop --*/

	// initialize variables
	OPTION_ID selectedOption;
	char inBuffer[512];
	int loop = 1;
	char *trimmed = NULL;

	// loop until `quit`
	while (loop) {
		// set default value to NONE
		selectedOption = NONE;

		// clear screen for options menu
		clearScreen();

		// print each command and its description
		for (int i = 0; i < COMMAND_COUNT; i++)
			printf("%s: %s\n", commands[i].command, commands[i].description);
		
		// read and normalize input
		fgets(inBuffer, sizeof(inBuffer), stdin);
		trimmed = trim(inBuffer);

		// check the input against each command
		for (int i = 0; i < COMMAND_COUNT; i++) {
			if (strcmp(commands[i].command, trimmed) == 0) {
				selectedOption = i;
				break;
			}
		}

		// clean up variables
		free(trimmed);
		trimmed = NULL;

		// perform the correct action for whichever option was chosen
		printf("\n");
		switch (selectedOption) {
			case load:
				printf("Provide the path of the program to load:\n");

				// read and normalize the inputted filepath
				fgets(inBuffer, sizeof(inBuffer), stdin);
				trimmed = trim(inBuffer);

				// aattempt to dd the program in that path to memory
				PROGRAM *newProgram = addProgram(trimmed, memory, &programMemory);

				// clean up
				free(trimmed);
				trimmed = NULL;

				// give a message and continue if it failed
				if (!newProgram) {
					printf("program couldn't be loaded into memory\n");
					break;
				}

				// otherwise give a message that it succeeded
				printf("Loaded program into memory at location %i with name \"%s\"\n", newProgram->startAddress, newProgram->name);
				break;

			case run:
				printf("Provide the name of the program to run:\n");

				// normalize the input
				fgets(inBuffer, sizeof(inBuffer), stdin);
				trimmed = trim(inBuffer);

				printf("Attempting to run program \"%s\"\n", trimmed);

				// cheack the provided name against each loaded program
				PROGRAM *loadedProgram = NULL;
				for (int i = 0; i < programMemory.size; i++) {
					if (strcmp(trimmed, ((PROGRAM *) listGetElement(&programMemory, i))->name) == 0) {
						loadedProgram = (PROGRAM *) listGetElement(&programMemory, i);
						break;
					}
				}

				// if none were found, error out
				if (!loadedProgram) {
					printf("Error loading program! Use the \"list\" command to list loaded programs\n");
					break;
				}

				// otherwise, give the location in memory
				printf("Program found at location %i\n", loadedProgram->startAddress);

				unsigned int pc = loadedProgram->startAddress;
				int loadHits = 0;
				int loadMisses = 0;
				int storeHits = 0;
				int storeMisses = 0;
				while (1) {
					// printf("[%4hi]: ", pc);
					// printMachineCode(memory[pc]);

					DECODED_INSTRUCTION decoded = decode(memory[pc]);
					if (execute(decoded, &pc, &loadHits, &loadMisses, &storeHits, &storeMisses)) break;

					pc++;
				}

				evictCache(cache, memory);

				printProgramStats(loadHits, loadMisses, storeHits, storeMisses);
				break;
				
			case purgep:
				// just run the helper function
				printf("Purging programs...\n");
				resetPrograms();
				printf("All program memory cleared!\n");
				break;
				
			case purged:
				// just run the helper function
				printf("Purging data...\n");
				resetData();
				printf("All non-program memory cleared!\n");
				break;
				
			case save:
				// just run the helper function
				printf("saving memory to ./output/disk.txt\n");
				dumpMemory("./output/disk.txt");
				break;

			case list:
				// just run the helper function
				printf("Listing every program name:\n");
				listMapFunction(&programMemory, *printProgramName);
				break;

			case quit:
				// set loop to zero
				printf("exiting\n");
				loop = 0;
				break;
			
			default:
				// bad input, give a message and move on to the next loop
				printf("this option is not supported\n");
		}

		// wait for input to allow the user to read before looping
		printf("\npress ENTER to continue\n");
		fgets(inBuffer, sizeof(inBuffer), stdin);
	}

	// clean up
	listClear(&programMemory);

	return 0;
}
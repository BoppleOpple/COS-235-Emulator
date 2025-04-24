#include "cache.h"
#include "assembler.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

CACHE_ROW createCacheRow(char flags) {
	return (CACHE_ROW) {
		flags,
		0
	};
}

CACHE *createCache() {
	CACHE *newCache = malloc(sizeof(CACHE) * CACHE_COUNT);

	resetCache(newCache);

	return newCache;
}

void resetCache(CACHE *cache) {
	for (int i = 0; i < CACHE_COUNT; i++) {
		for (int j = 0; j < CACHE_BLOCKS; j++) {
			// cache 0 stalest, cache CACHE_COUNT-1 freshest
			cache[i].rows[j].flags = ( CACHE_COUNT - i - 1 ) << 2;
		}
	}
}

void evictCache(CACHE *cache, int *memory) {
	for (int i = 0; i < CACHE_COUNT; i++)
		for (int j = 0; j < CACHE_BLOCKS; j++)
			cacheEvictBlock(cache, memory, i, j);
}

void cacheEvictBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow) {
	const int blockAddress = ( ( cache[cacheIndex].rows[cacheRow].tag << CACHE_INDEX_FIELD_SIZE ) + cacheRow ) << CACHE_BLOCK_FIELD_SIZE;

	if ( cache[cacheIndex].rows[cacheRow].flags & 0b001 ) {
		for (int i = 0; i < CACHE_BLOCK_SIZE; i++)
			memory[blockAddress + i] = cache[cacheIndex].rows[cacheRow].block[i];
	}

	cache[cacheIndex].rows[cacheRow].flags = ( CACHE_COUNT - cacheIndex - 1 ) << 2;
}

void cacheLoadBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow, int tag) {
	if (cache[cacheIndex].rows[cacheRow].flags & 0b010)
		cacheEvictBlock(cache, memory, cacheIndex, cacheRow);

	for (int i = 0; i < CACHE_COUNT; i++)
		cache[i].rows[cacheRow].flags += 0b100;

	cache[cacheIndex].rows[cacheRow].tag = tag;
	cache[cacheIndex].rows[cacheRow].flags = 0b010;

	const int blockAddress = ( tag << ( CACHE_INDEX_FIELD_SIZE + CACHE_BLOCK_FIELD_SIZE ) ) + ( cacheRow << CACHE_BLOCK_FIELD_SIZE );
	for (int i = 0; i < CACHE_BLOCK_SIZE; i++)
		cache[cacheIndex].rows[cacheRow].block[i] = memory[blockAddress + i];
}

int cacheLoadWord(CACHE *cache, int *memory, int address, int *hits, int *misses) {
	const unsigned int blockOffset = address & ( ( (unsigned) -1 << CACHE_BLOCK_FIELD_SIZE ) ^ -1 );
	const unsigned int index = (address >> CACHE_BLOCK_FIELD_SIZE) & ( ( ( (unsigned) -1 << CACHE_INDEX_FIELD_SIZE ) ^ -1 ) );
	const unsigned int tag = address >> ( CACHE_INDEX_FIELD_SIZE + CACHE_BLOCK_FIELD_SIZE );

	int staleCacheIndex = 0;
	int selectedCache = -1;


	for (int i = 0; i < CACHE_COUNT; i++) {
		if ( ( cache[i].rows[index].flags >> 2 ) > ( cache[staleCacheIndex].rows[index].flags >> 2 ) )
			staleCacheIndex = i;
		
		if ( ( cache[i].rows[index].flags & 0b010 ) && ( cache[i].rows[index].tag == tag ) ) {
			selectedCache = i;
			(*hits)++;
			break;
		}
	}

	if (selectedCache == -1) {
		cacheLoadBlock(cache, memory, staleCacheIndex, index, tag);
		selectedCache = staleCacheIndex;
		(*misses)++;
	}

	return cache[selectedCache].rows[index].block[blockOffset];
}

void cacheStoreWord(CACHE *cache, int *memory, int address, int *hits, int *misses, int data) {
	const unsigned int blockOffset = address & ( ( (unsigned) -1 << CACHE_BLOCK_FIELD_SIZE ) ^ -1 );
	const unsigned int index = (address >> CACHE_BLOCK_FIELD_SIZE) & ( ( ( (unsigned) -1 << CACHE_INDEX_FIELD_SIZE ) ^ -1 ) );
	const unsigned int tag = address >> ( CACHE_INDEX_FIELD_SIZE + CACHE_BLOCK_FIELD_SIZE );
	
	cacheLoadWord(cache, memory, address, hits, misses);

	for (int i = 0; i < CACHE_COUNT; i++) {
		if ( ( cache[i].rows[index].flags & 0b010 ) && ( cache[i].rows[index].tag == tag ) ) {
			cache[i].rows[index].block[blockOffset] = data;
			cache[i].rows[index].flags |= 0b001;
			break;
		}
	}
}

void printCacheToFile(CACHE *cache, FILE *f) {
	fprintf(f, "CACHE\n");
	for (int row = 0; row < CACHE_BLOCKS; row++) {
		for (int cacheIndex = 0; cacheIndex < CACHE_COUNT; cacheIndex++) {
			const char stale = cache[cacheIndex].rows[row].flags >> 2;
			const char *valid = ( ( cache[cacheIndex].rows[row].flags >> 1 ) & 1 ) ? "  VALID" : "INVALID" ;
			const char *dirty = ( cache[cacheIndex].rows[row].flags & 1 ) ? "DIRTY" : "CLEAN";

			fprintf(f, "%2i/%i[%i, %s, %s]: ", row, cacheIndex, stale, valid, dirty);

			for (int i = 0; i < CACHE_BLOCK_SIZE; i++) {
				fprintf(f, "%4i ", cache[cacheIndex].rows[row].block[i]);
			}

			fprintf(f, "\n");
		}
	}
}
#include "cache.h"
#include "assembler.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

CACHE_ROW createCacheRow(char flags) {
	// default tag to 0
	return (CACHE_ROW) {
		flags,
		0
	};
}

CACHE *createCache() {
	CACHE *newCache = malloc(sizeof(CACHE) * CACHE_COUNT);

	// resetting the cache is handled sseperately
	resetCache(newCache);

	return newCache;
}

void resetCache(CACHE *cache) {
	for (int i = 0; i < CACHE_COUNT; i++) {
		for (int j = 0; j < CACHE_BLOCKS; j++) {
			// this invalidates a row of cache and sets the stale bit(s)
			// cache 0 stalest, cache CACHE_COUNT-1 freshest
			cache[i].rows[j].flags = ( CACHE_COUNT - i - 1 ) << 2;
		}
	}
}

void evictCache(CACHE *cache, int *memory) {
	// simply iterate through cache and evict blocks
	for (int i = 0; i < CACHE_COUNT; i++)
		for (int j = 0; j < CACHE_BLOCKS; j++)
			cacheEvictBlock(cache, memory, i, j);
}

void cacheEvictBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow) {
	const int blockAddress = ( ( cache[cacheIndex].rows[cacheRow].tag << CACHE_INDEX_FIELD_SIZE ) + cacheRow ) << CACHE_BLOCK_FIELD_SIZE;

	// save data to memory if dirty
	if ( cache[cacheIndex].rows[cacheRow].flags & 0b001 ) {
		for (int i = 0; i < CACHE_BLOCK_SIZE; i++)
			memory[blockAddress + i] = cache[cacheIndex].rows[cacheRow].block[i];
	}

	// set the stale bit back to default and invalidate the row
	cache[cacheIndex].rows[cacheRow].flags = ( CACHE_COUNT - cacheIndex - 1 ) << 2;
}

void cacheLoadBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow, int tag) {
	// first, evict the block if it is valid
	if (cache[cacheIndex].rows[cacheRow].flags & 0b010)
		cacheEvictBlock(cache, memory, cacheIndex, cacheRow);
	
	// next, increment stale bits for other caches
	for (int i = 0; i < CACHE_COUNT; i++)
		cache[i].rows[cacheRow].flags += 0b100;
	
	// finally, set the tag, flags, and data of the row
	cache[cacheIndex].rows[cacheRow].tag = tag;
	cache[cacheIndex].rows[cacheRow].flags = 0b010;

	const int blockAddress = ( tag << ( CACHE_INDEX_FIELD_SIZE + CACHE_BLOCK_FIELD_SIZE ) ) + ( cacheRow << CACHE_BLOCK_FIELD_SIZE );
	for (int i = 0; i < CACHE_BLOCK_SIZE; i++)
		cache[cacheIndex].rows[cacheRow].block[i] = memory[blockAddress + i];
}

int cacheLoadWord(CACHE *cache, int *memory, int address, int *hits, int *misses) {
	// extract important values from the address
	const unsigned int blockOffset = address & ( ( (unsigned) -1 << CACHE_BLOCK_FIELD_SIZE ) ^ -1 );
	const unsigned int index = (address >> CACHE_BLOCK_FIELD_SIZE) & ( ( ( (unsigned) -1 << CACHE_INDEX_FIELD_SIZE ) ^ -1 ) );
	const unsigned int tag = address >> ( CACHE_INDEX_FIELD_SIZE + CACHE_BLOCK_FIELD_SIZE );

	// store the index of the stalest cache
	int staleCacheIndex = 0;
	// default to an invalid cache
	int selectedCache = -1;


	// iterate through caches to find either a matching address or the stalest cache
	for (int i = 0; i < CACHE_COUNT; i++) {
		if ( ( cache[i].rows[index].flags >> 2 ) > ( cache[staleCacheIndex].rows[index].flags >> 2 ) )
			staleCacheIndex = i;
		
		// if the address is found and is valid, reference that cache (cache hit)
		if ( ( cache[i].rows[index].flags & 0b010 ) && ( cache[i].rows[index].tag == tag ) ) {
			selectedCache = i;
			(*hits)++;
			break;
		}
	}

	// if there was no cache hit, replace the stalest cache (cache miss)
	if (selectedCache == -1) {
		cacheLoadBlock(cache, memory, staleCacheIndex, index, tag);
		selectedCache = staleCacheIndex;
		(*misses)++;
	}

	// return the value
	return cache[selectedCache].rows[index].block[blockOffset];
}

void cacheStoreWord(CACHE *cache, int *memory, int address, int *hits, int *misses, int data) {
	// extract important values from the address
	const unsigned int blockOffset = address & ( ( (unsigned) -1 << CACHE_BLOCK_FIELD_SIZE ) ^ -1 );
	const unsigned int index = (address >> CACHE_BLOCK_FIELD_SIZE) & ( ( ( (unsigned) -1 << CACHE_INDEX_FIELD_SIZE ) ^ -1 ) );
	const unsigned int tag = address >> ( CACHE_INDEX_FIELD_SIZE + CACHE_BLOCK_FIELD_SIZE );
	
	// load into cache to ensure the data is present
	cacheLoadWord(cache, memory, address, hits, misses);

	// find the correct address and set both the data and the dirty bit
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
	// iterate through each row of each cache
	for (int row = 0; row < CACHE_BLOCKS; row++) {
		for (int cacheIndex = 0; cacheIndex < CACHE_COUNT; cacheIndex++) {
			const char stale = cache[cacheIndex].rows[row].flags >> 2;
			const char *valid = ( ( cache[cacheIndex].rows[row].flags >> 1 ) & 1 ) ? "  VALID" : "INVALID" ;
			const char *dirty = ( cache[cacheIndex].rows[row].flags & 1 ) ? "DIRTY" : "CLEAN";

			// print the staleness, validity, and dirtiness of each element
			fprintf(f, "%2i/%i[%i, %s, %s]: ", row, cacheIndex, stale, valid, dirty);

			// as well as its block data
			for (int i = 0; i < CACHE_BLOCK_SIZE; i++) {
				fprintf(f, "%4i ", cache[cacheIndex].rows[row].block[i]);
			}

			fprintf(f, "\n");
		}
	}
}
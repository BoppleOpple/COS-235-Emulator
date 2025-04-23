#include "cache.h"
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

	for (int i = 0; i < CACHE_COUNT; i++)
		for (int j = 0; j < CACHE_BLOCKS; j++)
			newCache[i].rows[i] = createCacheRow(i << 2);

	return newCache;
}

void evictCache(CACHE *cache, int *memory) {
	for (int i = 0; i < CACHE_COUNT; i++)
		for (int j = 0; j < CACHE_BLOCKS; j++)
			cacheEvictBlock(cache, memory, i, j);
}

void cacheEvictBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow) {
	const int blockAddress = ( cache[cacheIndex].rows[cacheRow].tag << ( CACHE_INDEX_FIELD_SIZE + CACHE_BLOCK_FIELD_SIZE ) ) + ( cacheRow << CACHE_BLOCK_FIELD_SIZE );

	if ( cache[cacheIndex].rows[cacheRow].flags & 0b001 )
		for (int i = 0; i < CACHE_BLOCK_SIZE; i++)
			memory[blockAddress + i] = cache[cacheIndex].rows[cacheRow].block[i];

	cache[cacheIndex].rows[cacheRow].flags = 0b000;
}

void cacheLoadBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow, int tag) {
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
	const unsigned int index = address & ( ( ( (unsigned) -1 << CACHE_INDEX_FIELD_SIZE ) ^ -1 ) << CACHE_BLOCK_FIELD_SIZE );
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
	const unsigned int index = address & ( ( ( (unsigned) -1 << CACHE_INDEX_FIELD_SIZE ) ^ -1 ) << CACHE_BLOCK_FIELD_SIZE );
	const unsigned int tag = address >> ( CACHE_INDEX_FIELD_SIZE + CACHE_BLOCK_FIELD_SIZE );
	
	cacheLoadWord(cache, memory, address, hits, misses);

	for (int i = 0; i < CACHE_COUNT; i++) {
		if ( ( cache[index].rows[i].flags & 0b010 ) && ( cache[index].rows[i].tag == tag ) ) {
			cache[index].rows[i].block[blockOffset] = data;
			break;
		}
	}
}
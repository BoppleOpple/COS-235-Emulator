#pragma once

#define CACHE_BLOCK_FIELD_SIZE 2
#define CACHE_BLOCK_SIZE 1 << CACHE_BLOCK_FIELD_SIZE

#define CACHE_INDEX_FIELD_SIZE 4
#define CACHE_BLOCKS 1 << CACHE_INDEX_FIELD_SIZE

#define CACHE_COUNT 2

typedef struct {
	/**
	 * flags: 3 bit int with:
	 *     bit 0: dirty
	 *     bit 1: valid
	 *     bit 2: stale
	 */

	char flags;
	int tag;
	int block[CACHE_BLOCK_SIZE];
} CACHE_ROW;

typedef struct {
	CACHE_ROW rows[CACHE_BLOCKS];
} CACHE;

CACHE_ROW createCacheRow(char flags);

CACHE *createCache();

void evictCache(CACHE *cache, int *memory);

void cacheEvictBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow);

void cacheLoadBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow, int tag);

int cacheLoadWord(CACHE *cache, int *memory, int address, int *hits, int *misses);

void cacheStoreWord(CACHE *cache, int *memory, int address, int *hits, int *misses, int data);
#pragma once

#include <stdio.h>
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

/**
 * @brief Create a Cache Row struct with specified flags
 * 
 * @param flags 
 * @return CACHE_ROW 
 */
CACHE_ROW createCacheRow(char flags);

/**
 * @brief allocates and returns a list of associative caches
 * 
 * @return CACHE* 
 */
CACHE *createCache();

/**
 * @brief invalidates and sets stale bits for every element of cache
 * 
 * @param cache the list of caches to operate on
 */
void resetCache(CACHE *cache);

/**
 * @brief evicts every row in cache
 * 
 * @param cache the list of caches to evict
 * @param memory a pointer to emulator memory
 */
void evictCache(CACHE *cache, int *memory);

/**
 * @brief evicts a specific block of cache, saving to memory if dirty
 * 
 * @param cache the list of caches to evict
 * @param memory a pointer to emulator memory
 * @param cacheIndex the index of the cache containing the targeted row
 * @param cacheRow the index of the targeted row
 */
void cacheEvictBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow);

/**
 * @brief loads a specific block of cache
 * 
 * @param cache the list of caches to load into
 * @param memory a pointer to emulator memory
 * @param cacheIndex the index of the cache containing the targeted row
 * @param cacheRow the index of the targeted row
 * @param tag the tag of the data in memory
 */
void cacheLoadBlock(CACHE *cache, int *memory, int cacheIndex, int cacheRow, int tag);

/**
 * @brief loads a specific word from cache, pulling from memory if needed
 * 
 * @param cache the list of caches to load from
 * @param memory a pointer to emulator memory
 * @param address the address of the data in memory
 * @param hits a pointer to the cache hit counter
 * @param misses a pointer to the cache miss counter
 * @return int 
 */
int cacheLoadWord(CACHE *cache, int *memory, int address, int *hits, int *misses);

/**
 * @brief stores a specific word into cache, pulling from memory if needed
 * 
 * @param cache the list of caches to store to
 * @param memory a pointer to emulator memory
 * @param address the target address in memory
 * @param hits a pointer to the cache hit counter
 * @param misses a pointer to the cache miss counter
 * @param data the data to store
 */
void cacheStoreWord(CACHE *cache, int *memory, int address, int *hits, int *misses, int data);

/**
 * @brief prints cache information to a given file pointer
 * 
 * @param cache the list of caches to print
 * @param f the open and writable file
 */
void printCacheToFile(CACHE *cache, FILE *f);
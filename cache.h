#ifndef CACHE_H
#define CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_CACHE_ENTRIES 21
#define MAX_PATH_LENGTH 1024

typedef struct
{
    char path[MAX_PATH_LENGTH];   
    char *data;                // Contenido cargado en memoria
    size_t size;               
    time_t mtime;              // Última modificación del archivo en disco
    int in_use;                // 0 = libre, 1 = usada
    time_t last_access;        
} CacheEntry;



void init_cache(CacheEntry *cache);

CacheEntry* find_in_cache(CacheEntry *cache, const char *path);

int is_cache_valid(CacheEntry *entry, const char *filepath);

char *load_file(const char *filepath, size_t *out_size, time_t *out_mtime);

CacheEntry* get_free_or_lru_entry(CacheEntry *cache);

void store_in_cache(CacheEntry *cache, const char *path, const char *filepath,
                    char *data, size_t size, time_t mtime);

#endif 

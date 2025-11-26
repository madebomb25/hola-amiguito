#ifndef CACHE_H
#define CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_CACHE_ENTRIES 10
#define MAX_PATH_LEN 256

typedef struct
{
    char path[MAX_PATH_LEN];   // Ruta tipo "/index.html"
    char *data;                // Contenido cargado en memoria
    size_t size;               // Tamaño del contenido
    time_t mtime;              // Última modificación del archivo en disco
    int in_use;                // 0 = libre, 1 = usada
    time_t last_access;        // Timestamp para LRU
} CacheEntry;

/* FUNCIONES DEL CACHE */

void alex_cache();

void init_cache(CacheEntry *cache);

CacheEntry* find_in_cache(CacheEntry *cache, const char *path);

int is_cache_valid(CacheEntry *entry, const char *filepath);

char *load_file(const char *filepath, size_t *out_size, time_t *out_mtime);

CacheEntry* get_free_or_lru_entry(CacheEntry *cache);

void store_in_cache(CacheEntry *cache, const char *path, const char *filepath,
                    char *data, size_t size, time_t mtime);

#endif // CACHE_H

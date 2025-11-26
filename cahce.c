#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/stat.h>



#define MAX_CACHE_ENTRIES 10
#define MAX_PATH_LEN 256

typedef struct
{
	char path[MAX_PATH_LEN]; // p.e. "/index.html"
    char *data;              // contenido del archivo en memoria
    size_t size;             // tamaño del contenido
    time_t mtime;            // última modificación en disco
    int in_use;              // 0 = libre, 1 = usada
    time_t last_access;      // para implementar LRU
} CacheEntry;

void alex_cache(){
    CacheEntry cache[MAX_CACHE_ENTRIES];
    init_cache(cache);
    CacheEntry *entry = find_in_cache(path);
    if (entry && is_cache_valid(entry, filepath)) {
        entry->last_access = time(NULL);
        // enviar cabeceras HTTP...
        write(client_sock, entry->data, entry->size);
        return;
    }
    else{
        size_t size;
        time_t mtime;
        char *data = load_file(cache, filepath, &size, &mtime);
        if (!data) {
            // devolver 404 o error
        } else {
            store_in_cache(path, filepath, data, size, mtime);
            // enviar cabeceras HTTP...
            write(client_sock, data, size);
        }
    }
}

void init_cache(CacheEntry *cache) {
    for (int i = 0; i < MAX_CACHE_ENTRIES; i++) {
        cache[i].in_use = 0;
        cache[i].data = NULL;
    }
}

CacheEntry* find_in_cache(CacheEntry *cache, const char *path) {
    for (int i = 0; i < MAX_CACHE_ENTRIES; i++) {
        if (cache[i].in_use && strcmp(cache[i].path, path) == 0) {
            return &cache[i];
        }
    }
    return NULL;
}

int is_cache_valid(CacheEntry *entry, const char *filepath) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        return 0; // no existe o error
    }
    return st.st_mtime == entry->mtime;
}

char *load_file(const char *filepath, size_t *out_size, time_t *out_mtime) {
    struct stat st;
    if (stat(filepath, &st) == -1) return NULL;

    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;

    char *buf = malloc(st.st_size);
    if (!buf) { fclose(f); return NULL; }

    fread(buf, 1, st.st_size, f);
    fclose(f);

    *out_size = st.st_size;
    *out_mtime = st.st_mtime;
    return buf;
}

CacheEntry* get_free_or_lru_entry(CacheEntry *cache) {
    // 1º buscar libre
    int idx_free = 0;
    int idx_lru = 0;
    int trobat = 0;
    for (int i = 0; i < MAX_CACHE_ENTRIES && !trobat; i++) {
        if (!cache[i].in_use) {
            idx_free = i;
            trobat = 1;
        }
        if (cache[i].last_access < cache[idx_lru].last_access) idx_lru = i;
    }
    if (trobat) return &cache[idx_free];
    else{
        free(cache[idx_lru].data);
        return &cache[idx_lru];
    }
}

void store_in_cache(CacheEntry *cache, const char *path, const char *filepath,
                    char *data, size_t size, time_t mtime) {
    CacheEntry *entry = get_free_or_lru_entry(cache);
    strncpy(entry->path, path, MAX_PATH_LEN-1);
    entry->path[MAX_PATH_LEN-1] = '\0';
    entry->data = data;
    entry->size = size;
    entry->mtime = mtime;
    entry->last_access = time(NULL);
    entry->in_use = 1;
}


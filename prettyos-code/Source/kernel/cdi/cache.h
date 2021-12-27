#ifndef CDI_CACHE_H
#define CDI_CACHE_H

#include "util/types.h"

struct cdi_cache {
    size_t block_size; // Groesse der Blocks, die der Cache verwaltet

    // OS-Spezifische Daten folgen...
};

// Block fuer direkten, schnellen Zugriff.
struct cdi_cache_block {
    uint64_t number;  // Blocknummer
    void*    data;    // Zeiger auf die Daten
    void*    private; // Pointer auf blkpriv_len Bytes, die vom Aufrufer benutzt werden koennen
};

// Typ fuer Cache-Callback zum einlesen eines Blocks.
typedef int (cdi_cache_read_block_t)(struct cdi_cache* cache, uint64_t block, size_t count, void* dest, void* prv);

// Typ fuer Cache-Callback zum schreiben eines Blocks.
typedef int (cdi_cache_write_block_t)(struct cdi_cache* cache, uint64_t block, size_t count, const void* src, void* prv);

/* Cache erstellen
   block_size:  Groesse der Blocks die der Cache verwalten soll
   blkpriv_len: Groesse der privaten Daten die fuer jeden Block alloziert werden und danach vom aurfrufer frei benutzt werden duerfen
   read_block:  Funktionspointer auf eine Funktion zum einlesen eines Blocks.
   write_block: Funktionspointer auf eine Funktion zum schreiben eines Blocks.
   prv_data:    Wird den Callbacks als letzter Parameter uebergeben
   return:      Pointer auf das Cache-Handle */
struct cdi_cache* cdi_cache_create(size_t block_size, size_t blkpriv_len, cdi_cache_read_block_t* read_block, cdi_cache_write_block_t* write_block, void* prv_data);

// Destroy cache
void cdi_cache_destroy(struct cdi_cache* cache);

/* Veraenderte Cache-Blocks auf die Platte schreiben
   return: 1 bei Erfolg, 0 im Fehlerfall */
int cdi_cache_sync(struct cdi_cache* cache);


/* Cache-Block holen. Dabei wird intern ein Referenzzaehler erhoeht, sodass der
   Block nicht freigegeben wird, solange er benutzt wird. Das heisst aber auch,
   dass der Block nach der Benutzung wieder freigegeben werden muss, da sonst
   die freien Blocks ausgehen.

   cache:    Cache-Handle
   blocknum: Blocknummer
   noread:   Wenn != 0 wird der Block nicht eingelesen falls er noch nicht im Speicher ist. Kann benutzt werden, wenn der Block
                 vollstaendig ueberschrieben werden soll.
   return:   Pointer auf das Block-Handle oder NULL im Fehlerfall */
struct cdi_cache_block* cdi_cache_block_get(struct cdi_cache* cache, uint64_t blocknum, int noread);

/* Cache-Block freigeben
   cache: Cache-Handle
   block: Block-Handle */
void cdi_cache_block_release(struct cdi_cache* cache, struct cdi_cache_block* block);

/* Cache-Block als veraendert markieren
   cache: Cache-Handle
   block: Block-Handle */
void cdi_cache_block_dirty(struct cdi_cache* cache, struct cdi_cache_block* block);

#endif

#ifndef MM_POOL_H
#define MM_POOL_H

#include <stddef.h>

/*
 * メモリ断片化を抑制するため、固定サイズのチャンクを連結した
 * リニアアロケータ形式のメモリプールを定義する。
 */

/* メモリプール内の単一チャンク構造体 */
typedef struct PoolChunk {
    unsigned char *data;   /* チャンクが保有するデータバッファへのポインタ */
    size_t size;           /* チャンク全体のサイズ */
    size_t used;           /* 現在の使用済み領域のサイズ（先頭からのオフセット） */
    struct PoolChunk *next;/* チェーン内の次チャンクへのポインタ */
} PoolChunk;

/* メモリプール全体を管理する構造体 */
typedef struct MemPool {
    size_t chunk_size;     /* デフォルトチャンクサイズ（必要に応じて拡張） */
    PoolChunk *chunks;     /* チャンクの連結リスト先頭 */
} MemPool;

/* メモリプールを指定サイズと初期チャンク数で生成 */
MemPool *mm_pool_create(size_t chunk_size, size_t initial_chunks);

/* メモリプール内のすべてのリソースを破棄 */
void mm_pool_destroy(MemPool *pool);

/* メモリプール内のすべてのチャンクをリセット（データは残す） */
void mm_pool_reset(MemPool *pool);

/* メモリプールからメモリ領域を割り当て */
void *mm_pool_alloc(MemPool *pool, size_t size);

/* メモリプール上で文字列を複製 */
char *mm_pool_strdup(MemPool *pool, const char *src);

#endif /* MM_POOL_H */

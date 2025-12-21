#include "mm_pool.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* アラインメント境界に合わせてサイズを切り上げる */
static size_t align_up(size_t value, size_t alignment) {
    return (value + alignment - 1U) & ~(alignment - 1U);
}

/* メモリプール用チャンク構造体を動的に確保 */
static PoolChunk *alloc_chunk(size_t chunk_size) {
    /* チャンク管理構造体の領域確保 */
    PoolChunk *chunk = (PoolChunk *)malloc(sizeof(PoolChunk));
    if (!chunk) {
        return NULL;
    }
    /* チャンクが保有するデータバッファを確保 */
    chunk->data = (unsigned char *)malloc(chunk_size);
    if (!chunk->data) {
        free(chunk);
        return NULL;
    }
    /* チャンクの初期化 */
    chunk->size = chunk_size;
    chunk->used = 0;
    chunk->next = NULL;
    return chunk;
}

/* メモリプールを生成する */
MemPool *mm_pool_create(size_t chunk_size, size_t initial_chunks) {
    /* デフォルトチャンクサイズを設定 */
    if (chunk_size == 0) {
        chunk_size = 4096U;
    }
    /* プール管理構造体を確保 */
    MemPool *pool = (MemPool *)malloc(sizeof(MemPool));
    if (!pool) {
        return NULL;
    }
    pool->chunk_size = chunk_size;
    pool->chunks = NULL;

    /* 初期チャンクを複数個作成して連結 */
    PoolChunk *head = NULL;
    PoolChunk **tail = &head;
    for (size_t i = 0; i < initial_chunks; ++i) {
        PoolChunk *chunk = alloc_chunk(chunk_size);
        if (!chunk) {
            mm_pool_destroy(pool);
            return NULL;
        }
        *tail = chunk;
        tail = &chunk->next;
    }
    pool->chunks = head;
    return pool;
}

/* メモリプールを破棄してすべての領域を解放 */
void mm_pool_destroy(MemPool *pool) {
    if (!pool) {
        return;
    }
    /* チャーンリスト全体を走査して各チャンクを解放 */
    PoolChunk *chunk = pool->chunks;
    while (chunk) {
        PoolChunk *next = chunk->next;
        free(chunk->data);
        free(chunk);
        chunk = next;
    }
    /* プール管理構造体の解放 */
    free(pool);
}

/* メモリプール内のすべてのチャンクをリセット */
void mm_pool_reset(MemPool *pool) {
    if (!pool) {
        return;
    }
    /* 各チャンクの使用位置を初期化（メモリは解放しない） */
    for (PoolChunk *chunk = pool->chunks; chunk; chunk = chunk->next) {
        chunk->used = 0;
    }
}

/* 指定されたサイズを確保できるチャンクを探す、なければ新規作成 */
static PoolChunk *ensure_chunk(MemPool *pool, size_t size) {
    /* 要求サイズをアラインメント境界に合わせる */
    const size_t alignment = sizeof(max_align_t);
    size = align_up(size, alignment);

    /* 既存チャンクから十分な空き領域があるものを探す */
    PoolChunk *chunk = pool->chunks;
    PoolChunk *prev = NULL;
    while (chunk) {
        if (chunk->size - chunk->used >= size) {
            return chunk;
        }
        prev = chunk;
        chunk = chunk->next;
    }

    /* 適切なサイズの新規チャンクを作成 */
    size_t chunk_size = pool->chunk_size;
    if (size > chunk_size) {
        chunk_size = size;
    }
    PoolChunk *new_chunk = alloc_chunk(chunk_size);
    if (!new_chunk) {
        return NULL;
    }
    /* チャーンリストに新規チャンクを追加 */
    if (prev) {
        prev->next = new_chunk;
    } else {
        pool->chunks = new_chunk;
    }
    return new_chunk;
}

/* メモリプールからメモリ領域を割り当て */
void *mm_pool_alloc(MemPool *pool, size_t size) {
    if (!pool || size == 0) {
        return NULL;
    }
    /* 要求サイズを収容できるチャンクを取得 */
    PoolChunk *chunk = ensure_chunk(pool, size);
    if (!chunk) {
        return NULL;
    }
    /* アラインメントを調整して実際のサイズを確定 */
    const size_t alignment = sizeof(max_align_t);
    size = align_up(size, alignment);
    /* チャンク内で利用可能な位置を取得 */
    unsigned char *ptr = chunk->data + chunk->used;
    /* チャンクの使用済み領域を更新 */
    chunk->used += size;
    return ptr;
}

/* メモリプール上で文字列を複製 */
char *mm_pool_strdup(MemPool *pool, const char *src) {
    if (!pool || !src) {
        return NULL;
    }
    /* 文字列の長さを取得して必要な領域を確保 */
    size_t len = strlen(src);
    char *dst = (char *)mm_pool_alloc(pool, len + 1U);
    if (!dst) {
        return NULL;
    }
    /* 文字列をコピーしてヌル終端 */
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

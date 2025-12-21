#include "mm_pool.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_basic_alloc() {
    MemPool *pool = mm_pool_create(128, 1);
    assert(pool);
    char *a = (char *)mm_pool_alloc(pool, 10);
    char *b = (char *)mm_pool_alloc(pool, 20);
    assert(a && b);
    strcpy(a, "hello");
    strcpy(b, "world");
    assert(strcmp(a, "hello") == 0);
    assert(strcmp(b, "world") == 0);
    mm_pool_destroy(pool);
}

static void test_grow_chunks() {
    MemPool *pool = mm_pool_create(64, 1);
    assert(pool);
    void *big = mm_pool_alloc(pool, 200);
    assert(big);
    void *small = mm_pool_alloc(pool, 10);
    assert(small);
    mm_pool_destroy(pool);
}

static void test_strdup_and_reset() {
    MemPool *pool = mm_pool_create(64, 1);
    assert(pool);
    char *msg = mm_pool_strdup(pool, "reset me");
    assert(msg && strcmp(msg, "reset me") == 0);
    mm_pool_reset(pool);
    char *after = mm_pool_strdup(pool, "after reset");
    assert(after && strcmp(after, "after reset") == 0);
    mm_pool_destroy(pool);
}

int main(void) {
    test_basic_alloc();
    test_grow_chunks();
    test_strdup_and_reset();
    printf("mm_pool tests passed\n");
    return 0;
}

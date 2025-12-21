CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2

TARGETS = test_http test_mm_pool

all: $(TARGETS)

http.o: http.c http.h mm_pool.h
mm_pool.o: mm_pool.c mm_pool.h

test_http: http.c http.h mm_pool.c mm_pool.h test_http.c
	$(CC) $(CFLAGS) -o $@ http.c mm_pool.c test_http.c

test_mm_pool: mm_pool.c mm_pool.h test_mm_pool.c
	$(CC) $(CFLAGS) -o $@ mm_pool.c test_mm_pool.c

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean

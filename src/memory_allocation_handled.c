#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <err.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct context_t {
  void* current_position;
  void* end_chunk_position;
} context_t;

context_t ctxt = {.current_position = NULL, .end_chunk_position = NULL};

void __attribute__((noinline)) allocate_new_chunk() {
  int pagesize = getpagesize();
  int chunk_size = (pagesize * 256);
  void* base_chunk_ptr;

  if (posix_memalign(&base_chunk_ptr, pagesize, chunk_size) != 0) {
    errx(1, "failed to allocate a chunk");
  }

  ctxt.current_position = base_chunk_ptr;
  ctxt.end_chunk_position = base_chunk_ptr + chunk_size;
}

void* small_alloc(size_t size) {
  if (ctxt.current_position + size > ctxt.end_chunk_position) {
    allocate_new_chunk();
  }
  void* block = ctxt.current_position;
  ctxt.current_position += size;
  return block;
}

typedef struct pair_t {
  void* a;
  void* b;
} pair_t;

pair_t* pair(void* a, void* b) {
  pair_t* pair = small_alloc(sizeof(pair_t));
  pair->a = a;
  pair->b = b;
  return pair;
}
void print_pair(pair_t* pair) {
  printf("(%p, %p)\n", pair->a, pair->b);
}
int main() {
  pair_t* a = pair(NULL, NULL);
  pair_t* b = pair(a, NULL);

  print_pair(a);
  print_pair(b);

  return 0;
}

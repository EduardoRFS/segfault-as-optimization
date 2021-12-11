#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <err.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
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
  void* end_chunk_ptr;

  if (posix_memalign(&base_chunk_ptr, pagesize, chunk_size + pagesize) != 0) {
    errx(1, "failed to allocate a chunk");
  }
  end_chunk_ptr = base_chunk_ptr + chunk_size;

  if (mprotect(end_chunk_ptr, pagesize, PROT_NONE) != 0) {
    errx(1, "failed to lock page");
  }

  ctxt.current_position = base_chunk_ptr;
  ctxt.end_chunk_position = end_chunk_ptr;
}

void* small_alloc(size_t size) {
  void* block = ctxt.current_position;
  void* new_current_position = ctxt.current_position + size;
  ctxt.current_position = new_current_position;
  asm volatile(
      "movq %1, %%rcx\n\t"       // rcx = new_current_position
      "movq %2, %%rax\n\t"       // rax = block
      "movq %3, %%rdx\n\t"       // rdx = size
      "test (%%rcx), %%rax\n\t"  // *new_current_position
      "mov %%rax, %0"            // block = rax
      : "=r"(block)
      : "r"(new_current_position), "r"(block), "r"(size)
      : "rax", "rcx", "rdx", "cc");
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

void sig_handler(int sig, siginfo_t* si, ucontext_t* uctxt) {
  if (sig != SIGSEGV) {
    errx(1, "invalid signal");
  }

  // TODO: ensure si_addr is NULL or in the protected page

  mcontext_t* mctxt = &uctxt->uc_mcontext;
  size_t size = mctxt->gregs[REG_RDX];
  void* block;

  allocate_new_chunk();
  block = ctxt.current_position;
  ctxt.current_position += size;

  mctxt->gregs[REG_RAX] = (long long int)block;
  mctxt->gregs[REG_RCX] = (long long int)ctxt.current_position;
}

void register_sig_handler() {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = (void (*)(int, siginfo_t*, void*))sig_handler;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGSEGV, &sa, NULL) == -1) {
    errx(1, "failed to register signal");
  }
}

int main() {
  register_sig_handler();

  pair_t* a = pair(NULL, NULL);
  pair_t* b = pair(a, NULL);

  print_pair(a);
  print_pair(b);

  return 0;
}

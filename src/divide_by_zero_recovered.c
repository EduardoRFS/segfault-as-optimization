#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
// TODO: how to say that this file is only compatible with x86_64 and Linux
#include <err.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <ucontext.h>

void sig_handler(int sig, siginfo_t *si, ucontext_t *uctxt) {
  if (sig != SIGFPE) {
    errx(1, "invalid signal");
  }
  mcontext_t *ctxt = &uctxt->uc_mcontext;

  // skip idiv %rcx
  ctxt->gregs[REG_RIP] = ctxt->gregs[REG_RIP] + 3;
  ctxt->gregs[REG_RAX] = 7;
}

void register_sig_handler() {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = (void (*)(int, siginfo_t *, void *))sig_handler;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGFPE, &sa, NULL) == -1) {
    errx(1, "failed to register signal");
  }
}

#include <stdio.h>

long divide(volatile long num, volatile long den) {
  // volatile is a good way to prevent optimizations
  volatile long dst = -1;

  // dst = num / den
  // this is in ASM to ensure registers and idiv size across compilers
  asm volatile(
      "xor %%rdx, %%rdx\n\t"  // rdx = 0
      "mov %1, %%rax\n\t"     // rax = num
      "mov %2, %%rcx\n\t"     // rcx = den
      "idiv %%rcx\n\n"        // rax = rdx:rax / rcx
      "mov %%rax, %0"         // dst = rax
      : "=r"(dst)
      : "r"(num), "r"(den)
      : "rax", "rcx", "rdx");

  return dst;
}
int main() {
  register_sig_handler();

  long num = 5;
  long den = 0;
  long dst = divide(num, den);

  printf("%ld / %ld = %ld", num, den, dst);

  return 0;
}

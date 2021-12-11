#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE

#include <err.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <ucontext.h>

typedef struct node_t {
  int value;
  struct node_t *next;
} node_t;

node_t tail(volatile node_t current) {
  // volatile is a good way to prevent optimizations
  return *current.next;
}

void sig_handler(int sig, siginfo_t *si, void *unused) {
  if (sig != SIGSEGV || si->si_addr != NULL) {
    errx(1, "invalid signal");
  }
  errx(1, "tail called on NULL");
}

void register_sig_handler() {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sig_handler;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGSEGV, &sa, NULL) == -1) {
    errx(1, "failed to register signal");
  }
}

int main() {
  register_sig_handler();

  volatile node_t node;
  node.value = 1;
  node.next = NULL;

  volatile node_t node_tail = tail(node);

  printf("lol: %d", node_tail.value);

  return 0;
}

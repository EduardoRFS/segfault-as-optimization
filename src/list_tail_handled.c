#include <err.h>
#include <stddef.h>
#include <stdio.h>

typedef struct node_t {
  int value;
  struct node_t *next;
} node_t;

node_t tail(volatile node_t current) {
  // volatile is a good way to prevent optimizations
  if (current.next == NULL) {
    errx(1, "tail called on NULL");
  }

  return *current.next;
}

int main() {
  volatile node_t node;
  node.value = 1;
  node.next = NULL;

  volatile node_t node_tail = tail(node);

  printf("lol: %d", node_tail.value);

  return 0;
}

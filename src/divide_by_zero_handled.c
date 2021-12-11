#include <stdio.h>

long divide(volatile long num, volatile long den) {
  // volatile is a good way to prevent optimizations
  volatile long dst = -1;

  if (den == 0) {
    dst = 7;
  } else {
    dst = num / den;
  }

  return dst;
}
int main() {
  long num = 5;
  long den = 0;
  long dst = divide(num, den);

  printf("%ld / %ld = %ld", num, den, dst);

  return 0;
}

CC=gcc
CFLAGS=-g -Wall -O3

.PRECIOUS: bin/%
bin/%: src/%.c
	$(CC) $(CFLAGS) -o $@ $^

%: bin/%
	./$^

clean:
	rm -f bin/*

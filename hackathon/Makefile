CC=gcc
CFLAGS=-Wall -Wextra -O2

pagesim: pagesim.c
	$(CC) $(CFLAGS) -o pagesim pagesim.c

test: pagesim
	bash tests/run_tests.sh

demo: pagesim
	./pagesim -d 250

clean:
	rm -f pagesim

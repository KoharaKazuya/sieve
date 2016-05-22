sieve: sieve.c
	@clang -O -o sieve sieve.c

.PHONY: clean
clean:
	@rm -f ./sieve

LINTER_OPTION = -Weverything -Wno-vla

sieve: sieve.c
	@clang -O -o sieve $(LINTER_OPTION) sieve.c

.PHONY: clean
clean:
	@rm -f ./sieve

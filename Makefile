all:
	cc -Wall -Wextra cpu.c test.c -o test

debug:
	cc -Wall -Wextra cpu.c test.c -o test -D DEBUG=1

clean:
	rm test

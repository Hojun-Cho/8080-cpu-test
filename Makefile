all:
	cc -Wall -Wextra cpu.c test.c -o test

clean:
	rm test

all:
	cc -Wall -Wextra cpu.c diss.c game.c -I . -o game

debug:
	cc -Wall -Wextra cpu.c diss.c cpu_tests/test.c -I . -o test -D DEBUG=1

clean:
	rm test

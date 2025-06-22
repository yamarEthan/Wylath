all:
	gcc -Wall wylath.c display.c attacks.c moves.c -o wylath

clean:
	rm -f wylath
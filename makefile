all:
	gcc -Wall wylath.c display.c attacks.c -o wylath

clean:
	rm -f wylath
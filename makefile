all:
	gcc -Wall wylath.c display.c -o wylath

clean:
	rm -f wylath
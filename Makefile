consolepipe: consolepipe.c
	$(CC) $(CFLAGS) -oconsolepipe -lcurses consolepipe.c

clean:
	rm -f consolepipe

.PHONY: clean


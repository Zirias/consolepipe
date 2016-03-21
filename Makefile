BINS:= consolepipe pipewrite

all: $(BINS)

consolepipe: consolepipe.c
	$(CC) $(CFLAGS) -oconsolepipe -lcurses consolepipe.c

pipewrite: pipewrite.c
	$(CC) $(CFLAGS) -opipewrite -lcurses pipewrite.c

strip: $(BINS)
	strip --strip-all $(BINS)

clean:
	rm -f $(BINS)

.PHONY: all strip clean


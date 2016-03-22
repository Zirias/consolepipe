BINS:= bin/consolepipe bin/pipewrite

all: $(BINS)

bin/consolepipe: src/consolepipe.c src/usockservice.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o$@ -lcurses $>

bin/pipewrite: src/pipewrite.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o$@ $>

strip: $(BINS)
	strip --strip-all $(BINS)

clean:
	rm -fr bin

.PHONY: all strip clean


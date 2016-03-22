BINS:= bin/xcons_service bin/xcons_curses

all: $(BINS)

bin/xcons_curses: src/xcons_curses.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o$@ -lcurses $>

bin/xcons_service: src/xcons_service.c src/usockservice.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o$@ $>

strip: $(BINS)
	strip --strip-all $(BINS)

clean:
	rm -fr bin

.PHONY: all strip clean


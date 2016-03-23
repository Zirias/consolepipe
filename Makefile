BINS:= bin/xcons_service bin/xcons_curses

XCONS_CURSES_SRC:= src/xcons_curses.c
XCONS_SERVICE_SRC:= src/xcons_service.c src/usockservice.c

all: $(BINS)

bin/xcons_curses: $(XCONS_CURSES_SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o$@ -lcurses $(XCONS_CURSES_SRC)

bin/xcons_service: $(XCONS_SERVICE_SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o$@ $(XCONS_SERVICE_SRC)

strip: $(BINS)
	strip --strip-all $(BINS)

clean:
	rm -fr bin

.PHONY: all strip clean


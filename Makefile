prefix?=/usr/local
bindir?=$(prefix)/bin

CC?=cc
CFLAGS?=-O2 -pipe
INSTALL?=install

DEFINES+=-Dprefix=$(prefix)

BINS:= bin/xcons_service bin/xcons_curses

curses_OBJS:= src/xcons_curses.o
service_OBJS:= src/xcons_service.o src/usockservice.o

OBJS:= $(curses_OBJS) $(service_OBJS)

V?=0
VCC_0=   @echo "  [CC]      $@"
VCCLD_0= @echo "  [CCLD]    $@"
VMD_0=   @echo "  [MD]      $@"
VR_0:=@
VCC=$(VCC_$(V))
VCCLD=$(VCCLD_$(V))
VMD=$(VMD_$(V))
VR=$(VR_$(V))

all: $(BINS)

.SUFFIXES: .o

.c.o:
	$(VCC)
	$(VR)$(CC) $(CFLAGS) $(DEFINES) -o$@ -c $<

bin:
	$(VMD)
	$(VR)mkdir -p bin

bin/xcons_curses: $(curses_OBJS) bin
	$(VCCLD)
	$(VR)$(CC) -o$@ -lcurses $(curses_OBJS)

bin/xcons_service: $(service_OBJS) bin
	$(VCCLD)
	$(VR)$(CC) -o$@ $(service_OBJS)

strip: $(BINS)
	strip --strip-all $(BINS)

install: $(BINS)
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 $(BINS) $(DESTDIR)$(bindir)

install-strip: strip install

clean:
	rm -f $(OBJS)

distclean: clean
	rm -fr bin

.PHONY: all strip install install-strip clean distclean


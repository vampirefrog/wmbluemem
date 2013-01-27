# Makefile

# Installation directory
PREFIX=/usr/local

# Use Xft for the menu
USE_XFT=yes

VERSION=0.13

PROG=wmbluemem
BINDIR=$(PREFIX)/bin
MANPREFIX=$(PREFIX)/share/man
MANDIR=$(MANPREFIX)/man1
MANUAL=$(PROG).1
OBJS=main.o menu.o
CFLAGS=-O2 -Wall -DVERSION=\"$(VERSION)\"

OS=$(shell uname -s)
ifeq ($(OS),Linux)
LIBS=-L/usr/X11R6/lib -lX11 -lXext -lXpm
OBJS+=mem_linux.o
endif

ifeq ($(OS),FreeBSD)
LIBS=-L/usr/X11R6/lib -lX11 -lXext -lXpm -lkvm
OBJS+=mem_freebsd.o
endif

ifeq ($(USE_XFT),yes)
CFLAGS += -DUSE_XFT $(shell pkg-config xft --cflags)
LIBS += $(shell pkg-config xft --libs)
endif

CC=gcc
RM=rm -rf
INST=install
MKDIR_P=mkdir -p

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LIBS)
	strip $(PROG)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	$(RM) *.o $(PROG) *~ *.bak *.BAK .xvpics
install: $(PROG) $(MANUAL)
	$(INST) -m 755 $(PROG) $(BINDIR)
	$(MKDIR_P) $(MANDIR)
	$(INST) -m 644 $(MANUAL) $(MANDIR)
uninstall:
	$(RM) $(BINDIR)/$(PROG)
	$(RM) $(MANDIR)/$(MANUAL)

main.o: main.c mem.h mwm.h menu.h pixmap.xpm icon.xpm common.c
mem_freebsd.o: mem_freebsd.c mem.h
mem_linux.o: mem_linux.c
menu.o: menu.c menu.h

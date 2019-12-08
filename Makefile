#
# FHeroes2 for the RetroFW
#
# by pingflood; 2019
#


TARGET = fheroes2/fheroes2.dge

CHAINPREFIX 	:= /opt/mipsel-RetroFW-linux-uclibc
CROSS_COMPILE 	:= $(CHAINPREFIX)/usr/bin/mipsel-linux-

CC 		:= $(CROSS_COMPILE)gcc
CXX 	:= $(CROSS_COMPILE)g++
STRIP 	:= $(CROSS_COMPILE)strip
AR 		:= $(CROSS_COMPILE)ar

SYSROOT		:= $(shell $(CC) --print-sysroot)
SDL_LIBS 	:= $(shell $(SYSROOT)/usr/bin/sdl-config --libs)
SDL_FLAGS 	:= $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)


# makefile
# project: Free Heroes2
#
WITHOUT_NETWORK=1

CFLAGS := $(CFLAGS) -Wall -fsigned-char -DWITHOUT_NETWORK
LIBS :=

ifdef DEBUG
CFLAGS := $(CFLAGS) -O0 -g -pedantic -DWITH_DEBUG
else
CFLAGS := $(CFLAGS) -O2
endif

ifndef WITHOUT_MIXER
CFLAGS := $(CFLAGS) -DWITH_MIXER
SDL_LIBS := $(SDL_LIBS) -lSDL_mixer
endif

ifndef WITHOUT_IMAGE
CFLAGS := $(CFLAGS) -DWITH_IMAGE $(shell $(SYSROOT)/usr/bin/libpng-config --cflags) -DWITH_ZLIB
SDL_LIBS := $(SDL_LIBS) -lSDL_image $(shell $(SYSROOT)/usr/bin/libpng-config --libs) -lz
endif


ifndef WITHOUT_UNICODE
CFLAGS := $(CFLAGS) -DWITH_TTF
SDL_LIBS := $(SDL_LIBS) -lSDL_ttf
endif

ifndef WITHOUT_NETWORK
CFLAGS := $(CFLAGS) -DWITH_NET
SDL_LIBS := $(SDL_LIBS) -lSDL_net
endif

ifndef WITHOUT_XML
CFLAGS := $(CFLAGS) -DWITH_XML
endif

ifndef WITHOUT_ZLIB
CFLAGS := $(CFLAGS) -DWITH_ZLIB
LIBS := $(LIBS) -lz
endif

ifndef WITHOUT_EDITOR
CFLAGS := $(CFLAGS) -DWITH_EDITOR
endif

ifdef RELEASE
CFLAGS := $(CFLAGS) -DBUILD_RELEASE
endif

CFLAGS := $(SDL_FLAGS) $(CFLAGS)
LIBS := $(SDL_LIBS) $(LIBS)

# ifeq ($(PLATFORM),)
# ifeq ($(OS),Windows_NT)
# PLATFORM := mingw
# else
# PLATFORM := unix
# endif
# endif

CFLAGS := $(CFLAGS) -DWITHOUT_MOUSE -O3 -lintl

# include Makefile.$(PLATFORM)
PREFIX=$(SYSROOT)

export CXX AR LINK WINDRES CFLAGS LIBS PLATFORM SYSROOT PREFIX WITHOUT_NETWORK

.PHONY: clean

all:
ifndef WITHOUT_XML
	$(MAKE) -C xmlccwrap
endif
	$(MAKE) -C engine
	$(MAKE) -C dist
ifdef WITH_TOOLS
	$(MAKE) -C tools
endif
ifndef WITHOUT_UNICODE
	$(MAKE) -C dist pot
endif

clean:
ifndef WITHOUT_XML
	$(MAKE) -C xmlccwrap clean
endif
ifdef WITH_TOOLS
	$(MAKE) -C tools clean
endif
	$(MAKE) -C dist clean
	$(MAKE) -C engine clean

ipk: all
	@rm -rf /tmp/.fheroes2-ipk/ && mkdir -p /tmp/.fheroes2-ipk/root/home/retrofw/games/fheroes2 /tmp/.fheroes2-ipk/root/home/retrofw/apps/gmenu2x/sections/games
	@cp -r fheroes2/data fheroes2/files fheroes2/maps fheroes2/fheroes2.cfg fheroes2/fheroes2.dge fheroes2/fheroes2.png fheroes2/fheroes2.man.txt /tmp/.fheroes2-ipk/root/home/retrofw/games/fheroes2
	@cp fheroes2/fheroes2.lnk /tmp/.fheroes2-ipk/root/home/retrofw/apps/gmenu2x/sections/games
	@sed "s/^Version:.*/Version: $$(date +%Y%m%d)/" fheroes2/control > /tmp/.fheroes2-ipk/control
	@cp fheroes2/conffiles /tmp/.fheroes2-ipk/
	@tar --owner=0 --group=0 -czvf /tmp/.fheroes2-ipk/control.tar.gz -C /tmp/.fheroes2-ipk/ control conffiles
	@tar --owner=0 --group=0 -czvf /tmp/.fheroes2-ipk/data.tar.gz -C /tmp/.fheroes2-ipk/root/ .
	@echo 2.0 > /tmp/.fheroes2-ipk/debian-binary
	@ar r fheroes2/fheroes2.ipk /tmp/.fheroes2-ipk/control.tar.gz /tmp/.fheroes2-ipk/data.tar.gz /tmp/.fheroes2-ipk/debian-binary

opk: all
	@mksquashfs \
	fheroes2/default.retrofw.desktop \
	fheroes2/fheroes2.dge \
	fheroes2/data \
	fheroes2/files \
	fheroes2/maps \
	fheroes2/fheroes2.cfg \
	fheroes2/fheroes2.png \
	fheroes2/fheroes2.man.txt \
	fheroes2/fheroes2.opk \
	-all-root -noappend -no-exports -no-xattrs

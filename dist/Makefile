# makefile
# project: Free Heroes2
#

ifeq ($(PREFIX),)
PREFIX := /usr/local
endif

TARGET := ../fheroes2/fheroes2.dge
LIBENGINE := ../engine/libengine.a
CFLAGS := $(CFLAGS) -I../engine

ifndef WITHOUT_XML
LIBENGINE := $(LIBENGINE) ../xmlccwrap/libxmlccwrap.a
CFLAGS := $(CFLAGS) -I../xmlccwrap
endif

ifdef PREFIX
CFLAGS := $(CFLAGS) -I$(PREFIX)/include
endif

RES	:=
ICOFILE	:= ../../image/fheroes2_32x32.ico
SVN	:= $(shell svnversion ..)
CFLAGS	:= $(CFLAGS) -DSVN_REVISION="\"$(SVN)\""

ifdef WITH_ICONS
RES	:= $(TARGET).res
endif

SOURCEROOT := ../src
SOURCEDIR  := $(shell ls $(SOURCEROOT))
POT        := $(TARGET).pot

SEARCH := $(addprefix $(SOURCEROOT)/, $(addsuffix /*.cpp, $(SOURCEDIR)))

all: $(TARGET)

$(TARGET): $(notdir $(patsubst %.cpp, %.o, $(wildcard $(SEARCH)))) $(LIBENGINE) $(RES)
	@echo "lnk: $@"
	$(CXX) $(CFLAGS) $^ $(RES) $(LIBS) -o $@

pot: $(wildcard $(SEARCH))
	@echo "gen: $(POT)"
	@xgettext -d $(TARGET) -C -k_ -o $(POT) $(wildcard $(SEARCH))

$(RES): $(ICOFILE)
	@echo "gen: $(RES)"
	@echo "id ICON \"$(ICOFILE)\"" > $(TARGET).rc
	$(WINDRES) $(TARGET).rc -O coff -o $(TARGET).res

VPATH := $(addprefix $(SOURCEROOT)/, $(SOURCEDIR))

%.o: %.cpp
	@echo "cxx: $@"
	@$(CXX) -c -MD $(addprefix -I$(SOURCEROOT)/, $(SOURCEDIR)) $< $(CFLAGS)

include $(wildcard *.d)

.PHONY: clean

clean:
	rm -f *.pot *.o *.d *.rc *.res *.exe $(TARGET)

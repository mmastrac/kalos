ROOTDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))/../..))
OUTDIR := $(ROOTDIR)/target
DOSOUTDIR := $(ROOTDIR)/target/dos

KALOS_SRCDIR := $(ROOTDIR)/src
KALOS_SOURCES := $(wildcard $(KALOS_SRCDIR)/*.c)
KALOS_OBJECTS := $(patsubst $(KALOS_SRCDIR)/%.c,$(DOSOUTDIR)/kalos/%.o,$(KALOS_SOURCES))

SRCDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(DOSOUTDIR)/%.o,$(SOURCES))

SPACE_CHAR := $(subst ,, )
COMMA_CHAR := ,

.PHONY: all clean

all: $(DOSOUTDIR)/compiler.exe $(DOSOUTDIR)/example.exe

clean:
	rm -rf $(DOSOUTDIR)

WATCOM_MODEL=m
CFLAGS=-c -march=i86 -Wall -Werror -std=c99 \
	-fno-writable-strings -fnonconst-initializers -fshort-enum -ffunction-sections \
	-mcmodel=$(WATCOM_MODEL) -O3 -frerun-optimizer -floop-optimize -funroll-loops -fno-stack-check

WATCOM=docker run --rm \
	-v $(ROOTDIR)/src:/src/ \
	-v $(DOSOUTDIR):/obj/ \
	-v $(OUTDIR):/out/ \
	-v $(SRCDIR):/src2/ \
	mmastrac/openwatcom:2021-05-01
CC=$(WATCOM) owcc

$(SRCDIR)/dos_dispatch.inc: $(OUTDIR)/compiler $(SRCDIR)/dos.kidl
	$(OUTDIR)/compiler dispatch $(SRCDIR)/dos.kidl $(SRCDIR)/dos_dispatch.inc

$(SRCDIR)/dos_script.inc: $(OUTDIR)/compiler $(SRCDIR)/dos.kidl
	$(OUTDIR)/compiler compile $(SRCDIR)/dos.kidl $(SRCDIR)/dos.kalos $(DOSOUTDIR)/dos_script.bin
	xxd -i < $(DOSOUTDIR)/dos_script.bin > $(SRCDIR)/dos_script.inc

$(DOSOUTDIR)/kalos/%.o: $(KALOS_SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -I/src /src/$(<:$(KALOS_SRCDIR)/%=%) -o /obj/$(@:$(DOSOUTDIR)/%=%)

$(DOSOUTDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/dos_dispatch.inc $(SRCDIR)/dos_script.inc
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -I/src /src2/$(<:$(SRCDIR)/%=%) -o /obj/$(@:$(DOSOUTDIR)/%=%)

$(DOSOUTDIR)/compiler.exe.lst: $(KALOS_OBJECTS) $(DOSOUTDIR)/kalos/compiler/compiler_main.o
	echo $(patsubst $(DOSOUTDIR)/%.o,/obj/%.o,$^) > $@

$(DOSOUTDIR)/example.exe.lst: $(KALOS_OBJECTS) $(OBJECTS)
	echo $(patsubst $(DOSOUTDIR)/%.o,/obj/%.o,$^) > $@

$(DOSOUTDIR)/%.exe: $(DOSOUTDIR)/%.exe.lst
	$(WATCOM) wlink \
		name /out/dos/$(notdir $@) \
		system dos \
		file $(subst $(SPACE_CHAR),$(COMMA_CHAR),$(shell cat $^)) \
		library clib$(WATCOM_MODEL) \
		option map=/out/dos/$(notdir $@).map \
		option showdead \
		option nodefaultlibs \
		option eliminate > /dev/null

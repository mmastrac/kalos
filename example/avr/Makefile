ROOTDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))/../..))
SRCDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
OUTDIR := $(ROOTDIR)/target
AVROUTDIR := $(ROOTDIR)/target/avr

.PHONY: all clean

all: $(AVROUTDIR)/avr-example

clean:
	rm -rf $(AVROUTDIR)

CFLAGS=-g -Os -DF_CPU=16000000UL -DLOG= -mmcu=atmega328p -ffunction-sections -Wl,-gc-sections 

$(SRCDIR)/avr_dispatch.inc: $(OUTDIR)/compiler $(SRCDIR)/avr.kidl
	@mkdir -p $(AVROUTDIR)
	$(OUTDIR)/compiler dispatch $(SRCDIR)/avr.kidl $(SRCDIR)/avr_dispatch.inc

$(SRCDIR)/avr_script.inc: $(OUTDIR)/compiler $(SRCDIR)/avr.kidl $(SRCDIR)/avr.kalos
	@mkdir -p $(AVROUTDIR)
	$(OUTDIR)/compiler compile $(SRCDIR)/avr.kidl $(SRCDIR)/avr.kalos $(AVROUTDIR)/avr_script.bin
	xxd -i < $(AVROUTDIR)/avr_script.bin > $(SRCDIR)/avr_script.inc

$(AVROUTDIR)/kalos/%.o: $(ROOTDIR)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(ROOTDIR)/src -c $< -o $@

$(AVROUTDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/avr_dispatch.inc $(SRCDIR)/avr_script.inc
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(ROOTDIR)/src -c $< -o $@

$(AVROUTDIR)/avr-example: $(AVROUTDIR)/main.o $(AVROUTDIR)/kalos/kalos_run.o $(AVROUTDIR)/kalos/_kalos_value.o $(AVROUTDIR)/kalos/_kalos_string_system.o $(AVROUTDIR)/kalos/_kalos_script.o
	$(CC) $(CFLAGS) $^ -o $@

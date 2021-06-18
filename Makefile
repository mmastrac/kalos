SRCDIR=src
TESTDIR=test
OUTDIR=target
OBJDIR=$(OUTDIR)/obj
DOS_OBJDIR=$(OBJDIR)/dos
TEST_OBJDIR=$(OBJDIR)/test
HOST_OBJDIR=$(OBJDIR)/host
CC := $(shell which clang || which gcc)

SOURCES=$(wildcard $(SRCDIR)/*.c)
TEST_SOURCES=$(wildcard $(TESTDIR)/*.c) $(wildcard $(TESTDIR)/unity/*.c)
TEST_LIB_OBJECTS=$(patsubst $(SRCDIR)/%.c,$(TEST_OBJDIR)/lib/%.o,$(SOURCES))
TEST_OBJECTS=$(TEST_LIB_OBJECTS) $(patsubst $(TESTDIR)/%.c,$(TEST_OBJDIR)/test/%.o,$(TEST_SOURCES))

is_term_set := $(shell echo $$TERM)
ifdef is_term_set
TPUT_SGR0=$(shell tput sgr0 || echo '')
TPUT_BOLD=$(shell tput bold || echo '')
TPUT_DIM=$(shell tput dim || echo '')
TPUT_BG_BLACK=$(shell tput setab 0 || echo '')
TPUT_FG_GREEN=$(shell tput setaf 2 || echo '')
endif

define color_sh
	printf "$(TPUT_SGR0)$(TPUT_FG_GREEN)[ %-5s ]$(TPUT_SGR0)$(TPUT_FG_GREEN)$(TPUT_BOLD) %-30s$(TPUT_SGR0) $(TPUT_DIM)(%s)$(TPUT_SGR0)\n" "$1" "$3" "$2"
endef

define color
	@printf "$(TPUT_SGR0)$(TPUT_FG_GREEN)[ %-5s ]$(TPUT_SGR0)$(TPUT_FG_GREEN)$(TPUT_BOLD) %-30s$(TPUT_SGR0) $(TPUT_DIM)(%s)$(TPUT_SGR0)\n" "$1" "$3" "$2"
endef

WATCOM_MODEL=m
WATCOM_CFLAGS=-march=i86 -Wall -Werror -std=c99 \
	-fno-writable-strings -fnonconst-initializers -fshort-enum -ffunction-sections \
	-mcmodel=$(WATCOM_MODEL) -O3 -frerun-optimizer -floop-optimize -funroll-loops -fno-stack-check
#TODO: re-add -fmacro-backtrace-limit=0
TEST_CFLAGS=-std=c99 \
	-Werror -Wpedantic -Wno-typedef-redefinition -Wno-c11-extensions -Wno-gnu-flexible-array-initializer -ftrapv \
	-DIS_TEST -g -O0 \
	-fsanitize=undefined -fsanitize=address -fsanitize=integer -fsanitize=bounds \
	-fno-omit-frame-pointer
HOST_CFLAGS=-std=c99 \
	-Werror -Wpedantic -Wno-typedef-redefinition -Wno-c11-extensions -Wno-gnu-flexible-array-initializer -ftrapv \
	-g -O0 \
	-fsanitize=undefined -fsanitize=address -fsanitize=integer -fsanitize=bounds \
	-fno-omit-frame-pointer

.PHONY: test

test: $(OUTDIR)/tests/test
	$(call color,"TEST","host",$<)
	@$(OUTDIR)/tests/test

clean:
	$(call color,"CLEAN","all",$(OUTDIR))
	@rm -rf $(OUTDIR)

$(SRCDIR)/kalos_lex.c: $(SRCDIR)/kalos_lex.re
	$(call color,"re2c","all",$<)
	@re2c -W -c --tags --no-debug-info $< -o $@

$(SRCDIR)/kalos_string_format.c: $(SRCDIR)/kalos_string_format.re
	$(call color,"re2c","all",$<)
	@re2c -W -Wno-nondeterministic-tags -Wno-match-empty-string -c --tags --no-debug-info $< -o $@

$(SRCDIR)/kalos_idl_parse.c: $(SRCDIR)/kalos_idl_parse.re
	$(call color,"re2c","all",$<)
	@re2c -W -c --tags --no-debug-info $< -o $@

$(TEST_OBJDIR)/lib/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(call color,"CC","host",$<)
	@mkdir -p $(dir $@)
	@$(CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_OBJDIR)/test/%.o: $(TESTDIR)/%.c $(TESTDIR)/*.h $(TESTDIR)/*.inc $(HEADERS) Makefile
	$(call color,"CC","host",$<)
	@mkdir -p $(dir $@)
	@$(CC) $(TEST_CFLAGS) -Isrc -c $< -o $@

$(OUTDIR)/tests/test: $(TEST_OBJECTS)
	$(call color,"LINK","host",$@)
	@mkdir -p $(dir $@)
	@$(CC) $(TEST_CFLAGS) $(TEST_OBJECTS) -o $@

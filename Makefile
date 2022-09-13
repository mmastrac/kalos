OS := $(shell uname | tr '[:upper:]' '[:lower:]')
CC := $(shell which clang || which gcc)

SRCDIR=src
TESTDIR=test
ROOTOUTDIR=target
OUTDIR=$(ROOTOUTDIR)/$(OS)
GENDIR=$(OUTDIR)/gen
OBJDIR=$(OUTDIR)/obj
DOS_OBJDIR=$(OBJDIR)/dos
TEST_OBJDIR=$(OBJDIR)/test
HOST_OBJDIR=$(OBJDIR)/host
BOOTSTRAP_COMPILER=$(OUTDIR)/compiler-bootstrap
BOOTSTRAP_DIR=$(SRCDIR)/_bootstrap
BOOTSTRAP_CANDIDATE_DIR=$(OUTDIR)/bootstrap_candidate
BOOTSTRAP_CANDIDATE_SRCDIR=$(OUTDIR)/bootstrap_candidate/src
BOOTSTRAP_CANDIDATE_COMPILER=$(BOOTSTRAP_CANDIDATE_DIR)/compiler

SOURCES=$(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/modules/*.c) $(wildcard $(SRCDIR)/compiler/*.c)
HEADERS=$(wildcard $(SRCDIR)/*.h)
HOST_OBJECTS=$(patsubst $(SRCDIR)/%.c,$(HOST_OBJDIR)/%.o,$(SOURCES))
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
	-Werror -Wpedantic -Wno-typedef-redefinition -Wno-c11-extensions -Wno-overlength-strings -Wno-gnu-flexible-array-initializer -ftrapv \
	-DIS_TEST -g -O0 \
	-fsanitize=undefined -fsanitize=address -fsanitize=integer -fsanitize=bounds \
	-fno-omit-frame-pointer
HOST_CFLAGS=-std=c99 \
	-Werror -Wpedantic -Wno-typedef-redefinition -Wno-c11-extensions -Wno-overlength-strings -Wno-gnu-flexible-array-initializer -ftrapv \
	-g -O0 -DDEBUG=1 \
	-fsanitize=undefined -fsanitize=address -fsanitize=integer -fsanitize=bounds \
	-fno-omit-frame-pointer

.PHONY: all test clean linux-test linux-gen bootstrap

all: $(OUTDIR)/compiler

test: $(OUTDIR)/tests/test
	$(call color,"TEST","host",$<)
	@$(OUTDIR)/tests/test

linux-test:
	@docker run -it --rm -w /home/ -v `pwd`:/home/ silkeh/clang:latest make test

linux-gen:
	@docker run -it --rm -w /home/ -v `pwd`:/home/ silkeh/clang:latest make gen

clean:
	$(call color,"CLEAN","all",$(ROOTOUTDIR))
	@rm -rf $(ROOTOUTDIR)

$(HOST_OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) $(SRCDIR)/*.inc
	$(call color,"CC","host",$<)
	@mkdir -p $(dir $@)
	@$(CC) $(HOST_CFLAGS) -c $< -o $@

$(OUTDIR)/compiler: $(HOST_OBJECTS)
	$(call color,"LINK","host",$@)
	@mkdir -p $(dir $@)
	@$(CC) $(HOST_CFLAGS) $^ -o $@

$(SRCDIR)/_kalos_lex.c: $(SRCDIR)/_kalos_lex.re
	$(call color,"re2c","all",$<)
	@re2c -W -c --tags --no-debug-info $< -o $@ || echo "WARNING: re2c not installed. Not rebuilding $@."

$(SRCDIR)/_kalos_string_format.c: $(SRCDIR)/_kalos_string_format.re
	$(call color,"re2c","all",$<)
	@re2c -W -Wno-nondeterministic-tags -Wno-match-empty-string -c --tags --no-debug-info $< -o $@ || echo "WARNING: re2c not installed. Not rebuilding $@."

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

########################################################
# Self-hosting bits
########################################################

# Bootstrap requires a fully-tested bootstrap
$(BOOTSTRAP_COMPILER): $(BOOTSTRAP_DIR)/* $(BOOTSTRAP_DIR)/*/*
	$(call color,"BOOTSTRAP","host",$(BOOTSTRAP_COMPILER))
	@$(CC) $(HOST_CFLAGS) $(BOOTSTRAP_DIR)/{,compiler,modules}/*.c -o $(BOOTSTRAP_COMPILER)

bootstrap: $(BOOTSTRAP_COMPILER)

bootstrap-update: $(BOOTSTRAP_CANDIDATE_DIR)/success
	$(call color,"BOOTSTRAP","host",$(BOOTSTRAP_DIR))
	@rm -rf $(BOOTSTRAP_DIR) || true
	@cp -R $(BOOTSTRAP_CANDIDATE_SRCDIR) $(BOOTSTRAP_DIR)

# To successfully bootstrap, the bootstrap compiler must compile with its existing files, then compile with files
# that it generates itself.
$(BOOTSTRAP_CANDIDATE_DIR)/success: $(SRCDIR)/*.c $(SRCDIR)/compiler/*.c $(SRCDIR)/modules/*.c $(HEADERS)
	@rm -rf $(BOOTSTRAP_CANDIDATE_DIR)
	@mkdir -p $(BOOTSTRAP_CANDIDATE_SRCDIR)/{compiler,modules}
	@cp $(SRCDIR)/*.{c,h,inc} $(BOOTSTRAP_CANDIDATE_SRCDIR)
	@cp $(SRCDIR)/compiler/*.{c,h,inc} $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler
	@cp $(SRCDIR)/modules/*.{c,h} $(BOOTSTRAP_CANDIDATE_SRCDIR)/modules
	@# First, build with the existing generated files
	$(call color,"STG_0","host",$@)
	@$(CC) $(HOST_CFLAGS) $(BOOTSTRAP_CANDIDATE_SRCDIR)/{,compiler,modules}/*.c -o $(BOOTSTRAP_CANDIDATE_COMPILER)
	@# Invert success for help check
	@! $(BOOTSTRAP_CANDIDATE_COMPILER) > /dev/null
	$(call color,"STG_1","host",$@)
	@$(BOOTSTRAP_CANDIDATE_COMPILER) stringify $(SRCDIR)/compiler/compiler.kidl $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler.kidl.inc
	@$(BOOTSTRAP_CANDIDATE_COMPILER) stringify $(SRCDIR)/compiler/compiler.kalos $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler.kalos.inc
	@$(BOOTSTRAP_CANDIDATE_COMPILER) stringify $(SRCDIR)/compiler/compiler_idl.kalos $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler_idl.kalos.inc
	@$(BOOTSTRAP_CANDIDATE_COMPILER) dispatch $(SRCDIR)/compiler/compiler.kidl $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler.dispatch.inc > $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler.dispatch.inc
	@$(CC) $(HOST_CFLAGS) $(BOOTSTRAP_CANDIDATE_SRCDIR)/{,compiler,modules}/*.c -o $(BOOTSTRAP_CANDIDATE_COMPILER)
	$(call color,"STG_2","host",$@)
	@$(BOOTSTRAP_CANDIDATE_COMPILER) stringify $(SRCDIR)/compiler/compiler.kidl $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler.kidl.inc
	@$(BOOTSTRAP_CANDIDATE_COMPILER) stringify $(SRCDIR)/compiler/compiler.kalos $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler.kalos.inc
	@$(BOOTSTRAP_CANDIDATE_COMPILER) stringify $(SRCDIR)/compiler/compiler_idl.kalos $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler_idl.kalos.inc
	@$(BOOTSTRAP_CANDIDATE_COMPILER) dispatch $(SRCDIR)/compiler/compiler.kidl $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler.dispatch.inc > $(BOOTSTRAP_CANDIDATE_SRCDIR)/compiler/compiler.dispatch.inc
	@$(CC) $(HOST_CFLAGS) $(BOOTSTRAP_CANDIDATE_SRCDIR)/{,compiler,modules}/*.c -o $(BOOTSTRAP_CANDIDATE_COMPILER)
	@touch $@

gen: gen-dispatch gen-compiler gen-test

gen-test: $(OUTDIR)/compiler
	$(call color,"GEN","host","test")
	@$(OUTDIR)/compiler dispatch test/test_kalos.kidl $(OUTDIR)/test_kalos.dispatch.inc > $(OUTDIR)/test_kalos.dispatch.inc
	@mv $(OUTDIR)/test_kalos.dispatch.inc test/

gen-dispatch: $(OUTDIR)/compiler
	$(call color,"GEN","host","dispatch")
	@$(OUTDIR)/compiler dispatch src/_kalos_ops.kidl $(OUTDIR)/_kalos_ops.dispatch.inc > $(OUTDIR)/_kalos_ops.dispatch.inc
	@mv $(OUTDIR)/_kalos_ops.dispatch.inc src/

# TODO: dispatch generation is redirecting because we can't change where println outputs to yet
# We should consider making the IDL generation script write to a file rather than using println
gen-compiler: $(OUTDIR)/compiler
	$(call color,"GEN","host","compiler")
	@rm -rf $(GENDIR)/compiler || true
	@mkdir -p $(GENDIR)/compiler
	@cp -R $(SRCDIR)/* $(GENDIR)/compiler
	@$(OUTDIR)/compiler stringify src/compiler/compiler.kidl $(GENDIR)/compiler/compiler.kidl.inc
	@$(OUTDIR)/compiler stringify src/compiler/compiler.kalos $(GENDIR)/compiler/compiler.kalos.inc
	@$(OUTDIR)/compiler stringify src/compiler/compiler_idl.kalos $(GENDIR)/compiler/compiler_idl.kalos.inc
	@$(CC) $(HOST_CFLAGS) $(GENDIR)/compiler/*.c $(GENDIR)/compiler/modules/*.c $(GENDIR)/compiler/compiler/*.c -o $(OUTDIR)/bootstrap-compiler
	@$(OUTDIR)/bootstrap-compiler dispatch src/compiler/compiler.kidl $(GENDIR)/compiler/compiler.dispatch.inc > $(GENDIR)/compiler/compiler.dispatch.inc
	@cp $(GENDIR)/compiler/compiler.dispatch.inc $(SRCDIR)/compiler
	@cp $(GENDIR)/compiler/compiler.kidl.inc $(SRCDIR)/compiler
	@cp $(GENDIR)/compiler/compiler.kalos.inc $(SRCDIR)/compiler
	@cp $(GENDIR)/compiler/compiler_idl.kalos.inc $(SRCDIR)/compiler

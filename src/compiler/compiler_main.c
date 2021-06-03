#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include "../scripting/kalos_dump.h"
#include "../scripting/kalos_idl_compiler.h"
#include "../scripting/kalos_parse.h"

#define PAGE_ROUND_UP(offset, page_size) ((offset + page_size - 1) & (~(page_size - 1)))

int verbose = 0;

int help(const char* message, const char* cmd) {
    if (strchr(cmd, '/')) {
        cmd = strchr(cmd, '/') + 1;
    }
    if (strchr(cmd, '\\')) {
        cmd = strchr(cmd, '\\') + 1;
    }
    if (message) {
        printf("Error: %s\n\n", message);
    }
    printf("Usage:\n"
        "  %s [-v] compile idl-file source-file compiled-file\n"
        "  %s [-v] dump compiled-file\n"
        "  %s [-v] idl idl-source-file idl-file\n"
        "  %s [-v] dispatch idl-source-file c-file\n",
        cmd, cmd, cmd, cmd);
    return 1;
}

const char* read_file_string(const char* input, size_t* n) {
    FILE* fd = fopen(input, "rb");
    fseek(fd, 0, SEEK_END);
    off_t size = ftell(fd);
    char* buf = malloc(PAGE_ROUND_UP(size + 1, 16));
    fseek(fd, 0, SEEK_SET);
    fread((void*)buf, size, 1, fd);
    if (n) *n = (size_t)size;
    buf[size] = 0;
    fclose(fd);
    return buf;
}

void* read_file(const char* input, size_t* n) {
    FILE* fd = fopen(input, "rb");
    fseek(fd, 0, SEEK_END);
    off_t size = ftell(fd);
    char* buf = malloc(PAGE_ROUND_UP(size, 16));
    fseek(fd, 0, SEEK_SET);
    fread((void*)buf, size, 1, fd);
    if (n) *n = (size_t)size;
    fclose(fd);
    return buf;
}

void write_file(const char* output, void* data, size_t size) {
    FILE* fd = fopen(output, "wb");
    fwrite(data, size, 1, fd);
    fclose(fd);
}

int compile_script(int verbose, const char* idl, const char* input, const char* output) {
    const char* input_data = read_file_string(input, NULL);
    const char* idl_data = read_file_string(idl, NULL);
    kalos_script script = {0};
    script.script_ops = malloc(1024);
    script.script_buffer_size = 1024;
    kalos_module_parsed modules = kalos_idl_parse_module(idl_data);
    kalos_parse(input_data, modules, &script);
    write_file(output, script.script_ops, script.script_buffer_size);
    return 0;
}

int dump_script(int verbose, const char* input) {
    size_t n;
    uint8_t* input_data = read_file(input, &n);
    kalos_script script = { .script_ops = (uint8_t*)input_data, .script_buffer_size = n };
    char* buffer = malloc(10 * 1024);
    kalos_dump(&script, buffer);
    printf("%s\n", buffer);
    return 0;
}

int compile_idl(int verbose, const char* input, const char* output) {
    const char* input_data = read_file_string(input, NULL);
    kalos_module_parsed modules = kalos_idl_parse_module(input_data);
    write_file(output, modules.data, modules.size);
    return 0;
}

int compile_dispatch(int verbose, const char* input, const char* output) {
    const char* input_data = read_file_string(input, NULL);
    kalos_module_parsed modules = kalos_idl_parse_module(input_data);
    if (!modules.data) {
        printf("ERROR: failed to compile KIDL\n");
        exit(1);
    }
    if (!kalos_idl_generate_dispatch(modules)) {
        printf("ERROR: failed to generate dispatch\n");
        exit(2);
    }
    write_file(output, modules.data, modules.size);
    return 0;
}

void log_printf(const char* fmt, ...) {
    if (verbose) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        fputc('\n', stdout);
    }
}

#ifndef IS_TEST
int main(int argc, const char** argv) {
    int idx = 1;
    if (argc - idx <= 0) {
        return help(NULL, argv[0]);
    }
    if (strcmp(argv[idx], "-h") == 0) {
        return help(NULL, argv[0]);
    }
    if (strcmp(argv[idx], "-v") == 0) {
        verbose = 1;
        idx++;
    }
    if (argc - idx <= 0) {
        return help("missing mode", argv[0]);
    }
    const char* mode = argv[idx++];
    if (strcmp(mode, "idl") == 0) {
        if (argc - idx != 2) {
            return help("not enough arguments to idl", argv[0]);
        }
        const char* input = argv[idx++];
        const char* output = argv[idx++];
        return compile_idl(verbose, input, output);
    } else if (strcmp(mode, "dispatch") == 0) {
        if (argc - idx != 2) {
            return help("not enough arguments to dispatch", argv[0]);
        }
        const char* input = argv[idx++];
        const char* output = argv[idx++];
        return compile_dispatch(verbose, input, output);
    } else if (strcmp(mode, "compile") == 0) {
        if (argc - idx != 3) {
            return help("not enough arguments to compile", argv[0]);
        }
        const char* idl = argv[idx++];
        const char* input = argv[idx++];
        const char* output = argv[idx++];
        return compile_script(verbose, idl, input, output);
    } else if (strcmp(mode, "dump") == 0) {
        if (argc - idx != 1) {
            return help("not enough arguments to dump", argv[0]);
        }
        const char* input = argv[idx++];
        return dump_script(verbose, input);
    } else {
        return help("invalid mode", argv[0]);
    }

    return 0;
}
#endif

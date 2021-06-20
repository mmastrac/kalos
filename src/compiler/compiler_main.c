#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include "../_kalos_defines.h"
#include "../_kalos_idl_compiler.h"
#include "../kalos_parse.h"

#define PAGE_ROUND_UP(offset, page_size) ((offset + page_size - 1) & (~(page_size - 1)))

static int verbose = 0;
static FILE* output_file;

static int32_t total_allocated = 0;
static uint16_t allocation_id;
struct allocation_record {
    uint16_t id;
    size_t size;
};

static void* internal_malloc(size_t size) {
    total_allocated += size;
    LOG("alloc #%d %d", allocation_id, size);
    // Stash the size in the allocation
    struct allocation_record info;
    info.size = size;
    info.id = allocation_id++;
    uint8_t* allocated = malloc(size + sizeof(info));
    memcpy(allocated, &info, sizeof(info));
    return allocated + sizeof(info);
}

static void internal_free(void* memory) {
    uint8_t* allocated = memory;
    struct allocation_record info;
    allocated -= sizeof(info);
    memcpy(&info, allocated, sizeof(info));
    LOG("free #%d %d", info.id, info.size);
    total_allocated -= info.size;
    free(allocated);
}

static void* internal_realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return internal_malloc(size);
    }
    uint8_t* allocated = ptr;
    struct allocation_record info;
    allocated -= sizeof(info);
    memcpy(&info, allocated, sizeof(info));
    allocated = realloc(allocated, size + sizeof(info));
    total_allocated += (ssize_t)size - (ssize_t)info.size;
    info.size = size;
    memcpy(allocated, &info, sizeof(info));
    return allocated + sizeof(info);
}

static void internal_error(int line, const char* error) {
    if (line) {
        printf("ERROR on line %d: %s\n", line, error);
    } else {
        printf("ERROR: %s\n", error);
    }
    exit(1);
}

static void internal_print(const char* s) {
    fprintf(output_file, "%s", s);
}

kalos_state compiler_env = {
    .alloc = internal_malloc,
    .realloc = internal_realloc,
    .free = internal_free,
    .print = internal_print,
    .error = internal_error,
};

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
    if (!fd) {
        printf("ERROR: Failed to open %s\n", input);
        exit(1);
    }
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

kalos_buffer read_file(const char* input, size_t* n) {
    FILE* fd = fopen(input, "rb");
    fseek(fd, 0, SEEK_END);
    off_t size = ftell(fd);
    kalos_buffer buffer = kalos_buffer_alloc(&compiler_env, PAGE_ROUND_UP(size, 16));
    fseek(fd, 0, SEEK_SET);
    fread((void*)buffer.buffer, size, 1, fd);
    if (n) *n = (size_t)size;
    fclose(fd);
    return buffer;
}

void write_file(const char* output, void* data, size_t size) {
    FILE* fd = fopen(output, "wb");
    fwrite(data, size, 1, fd);
    fclose(fd);
}

int compile_script(int verbose, const char* idl, const char* input, const char* output) {
    const char* input_data = read_file_string(input, NULL);
    const char* idl_data = read_file_string(idl, NULL);
    kalos_script script = kalos_buffer_alloc(&compiler_env, 10 * 1024);
    kalos_module_parsed modules = kalos_idl_parse_module(idl_data, &compiler_env);
    kalos_parse_options options = {0};
    kalos_parse_result res = kalos_parse(input_data, modules, options, &script);
    if (!res.success) {
        printf("ERROR on line %d: %s\n", res.line, res.error);
        exit(1);
    }
    write_file(output, script.buffer, kalos_buffer_size(script));
    return 0;
}

int dump_script(int verbose, const char* input) {
    size_t n;
    kalos_script script = read_file(input, &n);
    char* buffer = malloc(10 * 1024);
    kalos_dump(&script, buffer);
    printf("%s\n", buffer);
    return 0;
}

int compile_idl(int verbose, const char* input, const char* output) {
    const char* input_data = read_file_string(input, NULL);
    kalos_module_parsed modules = kalos_idl_parse_module(input_data, &compiler_env);
    write_file(output, modules.buffer, kalos_buffer_size(modules));
    if (total_allocated != 0) {
        printf("WARNING: IDL compiler leaked %d bytes(s)\n", (int)total_allocated);
    }
    return 0;
}

int compile_dispatch(int verbose, const char* input, const char* output) {
    const char* input_data = read_file_string(input, NULL);
    kalos_module_parsed modules = kalos_idl_parse_module(input_data, &compiler_env);
    if (!modules.buffer) {
        printf("ERROR: failed to compile KIDL\n");
        exit(1);
    }
    output_file = fopen(output, "w");
    if (!kalos_idl_generate_dispatch(modules, &compiler_env)) {
        printf("ERROR: failed to generate dispatch\n");
        fclose(output_file);
        unlink(output);
        exit(2);
    }
    fclose(output_file);
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

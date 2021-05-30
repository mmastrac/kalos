#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include "../scripting/kalos_idl_compiler.h"
#include "../scripting/kalos_parse.h"

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
        "  %s [-v] compile input-file output-file\n"
        "  %s [-v] dump input-file\n"
        "  %s [-v] idl input-file output-file\n",
        cmd, cmd, cmd);
    return 1;
}

const char* read_file(const char* input) {
    FILE* fd = fopen(input, "rb");
    fseek(fd, 0, SEEK_END);
    off_t size = ftell(fd);
    char* buf = malloc(size + 1);
    fseek(fd, 0, SEEK_SET);
    fread((void*)buf, size, 1, fd);
    buf[size] = 0;
    return buf;
}

int compile_script(int verbose, const char* input, const char* output) {
    return 0;
}

int dump_script(int verbose, const char* input) {
    return 0;
}

int compile_idl(int verbose, const char* input, const char* output) {
    const char* input_data = read_file(input);
    kalos_idl_parse_module(input_data);
    return 0;
}

void log_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fputc('\n', stdout);
}

#ifndef IS_TEST
int main(int argc, const char** argv) {
    int verbose = 0;
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
    } else if (strcmp(mode, "compile") == 0) {
        if (argc - idx != 2) {
            return help("not enough arguments to compile", argv[0]);
        }
        const char* input = argv[idx++];
        const char* output = argv[idx++];
        return compile_script(verbose, input, output);
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

#include <stdio.h>

int help() {
    printf("Usage: %s [/v] input-file output-file \n", "fdlc.exe");
    return 1;
}

#ifndef IS_TEST
int main(int argc, const char** argv) {
    if (argc <= 2) {
        return help();
    }

    const char* input = argv[1];
    const char* output = argv[2];

    printf("%s %s\n", input, output);

    return 0;
}
#endif

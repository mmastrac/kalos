#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

test_suite* suites[256] = {0};
int test_suite_count = 0;

FILE* log_stream;

void register_suite(test_suite* current_suite) {
    // printf("suite %s\n", current_suite->name);
    suites[test_suite_count++] = current_suite;
}

void test_log(char* file, int line, const char* fmt, ...) {
    va_list args;
    static char print_buffer[4096];

    va_start(args, fmt);
    int written = vsnprintf(&print_buffer[0], sizeof(print_buffer), fmt, args);
    va_end(args);

    strcat(&print_buffer[0], "\n");
    fprintf(log_stream, "%s:%d: ", file, line);
    fwrite(&print_buffer[0], written + 1, 1, log_stream);
    fflush(log_stream);
}

void test_log_direct(const char* fmt, ...) {
    va_list args;
    static char print_buffer[2048];

    va_start(args, fmt);
    int written = vsnprintf(&print_buffer[0], sizeof(print_buffer), fmt, args);
    va_end(args);

    strcat(&print_buffer[0], "\n");
    fwrite(&print_buffer[0], written + 1, 1, log_stream);
    fflush(log_stream);
}

int main(void) {
    log_stream = fopen("/tmp/kalos-test.log", "wb");
    int failures = 0;

    for (int i = 0; i < test_suite_count; i++) {
        UnityPrint(suites[i]->file);
        UNITY_PRINT_EOL();
        UNITY_PRINT_EOL();
        UnityBegin(suites[i]->file);
        for (int j = 0; j < suites[i]->test_count; j++) {
            test_data* test_data = &suites[i]->tests[j];
            Unity.CurrentTestName = test_data->name;
            Unity.CurrentTestLineNumber = (UNITY_LINE_TYPE)test_data->line;
            Unity.NumberOfTests++;

            test_log_direct("");
            test_log_direct("***********************************************************");
            test_log_direct("* %s", test_data->name);
            test_log_direct("***********************************************************");
            test_log_direct("");

            UNITY_CLR_DETAILS();
            UNITY_EXEC_TIME_START();
            if (TEST_PROTECT())
            {
                // setUp();
                test_data->fn();
            }
            if (TEST_PROTECT())
            {
                // tearDown();
            }
            UNITY_EXEC_TIME_STOP();

            if (Unity.CurrentTestIgnored) {
                test_log_direct("IGNORED");                
            } else if (Unity.CurrentTestFailed) {
                test_log_direct("FAILED");
            } else {
                test_log_direct("PASSED");
            }

            UnityConcludeTest();
        }
        failures += UnityEnd();
        UNITY_PRINT_EOL();
    }

    if (failures) {
        UnityPrintNumber(failures);
        UnityPrint(" Failure(s)");
    } else {
        UnityPrint("All tests passed!");
    }
    UNITY_PRINT_EOL();

    fclose(log_stream);
}

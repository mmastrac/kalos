#include <stdlib.h>
#include "kalos_module_sys.h"
 
static int saved_argc;
static char** saved_argv;

void kalos_sys_set_args(int argc, char* argv[]) {
    saved_argc = argc;
    saved_argv = argv;
}

kalos_object_ref kalos_sys_get_args(kalos_run_state* state) {
    return NULL;
}

kalos_string kalos_sys_get_env(kalos_state* state, kalos_string* name) {
    return kalos_string_allocate_fmt(state, "%s", getenv(kalos_string_c(state, *name)));
}

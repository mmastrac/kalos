#pragma once
#include "../kalos_run.h"

static const kalos_int KALOS_FILE_READ_ONLY = 1;
static const kalos_int KALOS_FILE_WRITE_ONLY = 2;
static const kalos_int KALOS_FILE_READ_WRITE = 3;
static const kalos_int KALOS_FILE_CREATE = 4;
static const kalos_int KALOS_FILE_TRUNCATE = 8;
static const kalos_int KALOS_FILE_APPEND = 16;

kalos_object_ref kalos_file_open(kalos_state* state, kalos_string* file_, kalos_int mode_);
kalos_string kalos_file_readline(kalos_state* state, kalos_object_ref* object);
kalos_string kalos_file_read_all(kalos_state* state, kalos_object_ref* object);
kalos_string kalos_file_read(kalos_state* state, kalos_object_ref* object, kalos_int size);
kalos_int kalos_file_write(kalos_state* state, kalos_object_ref* object, kalos_string* data);
void kalos_file_close(kalos_state* state, kalos_object_ref* object);
kalos_object_ref kalos_file_get_stdin(kalos_state* state);
kalos_object_ref kalos_file_get_stdout(kalos_state* state);
kalos_object_ref kalos_file_get_stderr(kalos_state* state);

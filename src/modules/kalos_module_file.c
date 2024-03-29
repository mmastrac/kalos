#include <fcntl.h>
#include <unistd.h>
#include "../_kalos_defines.h"
#include "kalos_module_file.h"

#define FILE_BUFFER_SIZE 256
#define FILE_CLOSED -1

struct kalos_file {
    int fd;
    uint8_t ring_buffer[FILE_BUFFER_SIZE];
    bool closeable;
    uint16_t buffer_size;
    uint8_t buffer_start;
};

void kalos_file_free(kalos_state* state, kalos_object_ref* object) {
    struct kalos_file* file = (*object)->context;
    if (file->fd != FILE_CLOSED) {
        close(file->fd);
        file->fd = FILE_CLOSED;
    }
}

kalos_object_ref kalos_file_open_fd(kalos_state* state, int fd, bool closeable) {
    struct kalos_file file = {0};
    file.fd = fd;
    file.closeable = closeable;
    kalos_object_ref object = kalos_allocate_object(state, sizeof(file));
    object->object_free = kalos_file_free;
    memcpy(object->context, &file, sizeof(file));
    return object;
}

kalos_object_ref kalos_file_open(kalos_state* state, kalos_string* file_, kalos_int mode_) {
    int mode = 0;
    switch (mode_ & 3) {
        case KALOS_FILE_READ_ONLY: mode = O_RDONLY; break;
        case KALOS_FILE_WRITE_ONLY: mode = O_WRONLY; break;
        case KALOS_FILE_READ_WRITE: mode = O_RDWR; break;
        default: kalos_value_error(state); break;
    }
    if ((mode_ & KALOS_FILE_CREATE) == KALOS_FILE_CREATE) {
        mode |= O_CREAT;
    }
    if ((mode_ & KALOS_FILE_TRUNCATE) == KALOS_FILE_TRUNCATE) {
        mode |= O_TRUNC;
    }
    if ((mode & KALOS_FILE_APPEND) == KALOS_FILE_APPEND) {
        mode |= O_APPEND;
    }
    int fd = open(kalos_string_c(state, *file_), mode, 0644); // Always use mode (rw-r--r--)
    if (fd == -1) {
        kalos_value_error(state);
        return NULL;
    }
    return kalos_file_open_fd(state, fd, true);
}

kalos_int kalos_file_fill_buffer(struct kalos_file* file) {
    // Locate the end of the ring buffer and fill what we can from there to the buffer end
    int read_start = file->buffer_start;
    read_start = (read_start + file->buffer_size) % FILE_BUFFER_SIZE;
    int read_size = FILE_BUFFER_SIZE - max(read_start, file->buffer_size);
    if (read_size == -1) {
        return -1;
    }
    int r = read(file->fd, file->ring_buffer + read_start, read_size);
    if (r < 0) {
        return -1;
    }
    file->buffer_size = file->buffer_size + r;
    return r;
}

kalos_string kalos_file_read_to_delim(kalos_state* state, kalos_object_ref* object, char delim) {
    struct kalos_file* file = (*object)->context;
    if (file->fd == FILE_CLOSED) {
        kalos_value_error(state);
        return kalos_string_allocate(state, "");    
    }
    int size = 16;
    int length = 0;
    kalos_writable_string s = kalos_string_allocate_writable_size(state, size);
    char* buf = kalos_string_writable_c(state, s);
    for (;;) {
        while (file->buffer_size) {
            char c = file->ring_buffer[file->buffer_start];
            file->buffer_start = (file->buffer_start + 1) % FILE_BUFFER_SIZE;
            file->buffer_size--;
            if ((delim && c == delim) || length == KALOS_INT_MAX) {
                goto done;
            }
            if (length == size) {
                size *= 2;
                s = kalos_string_reallocate_writable(state, s, size * 2);
                buf = kalos_string_writable_c(state, s);
            }
            buf[length++] = c;
        }
        kalos_int fill = kalos_file_fill_buffer(file);
        if (fill == -1) {
            kalos_value_error(state);
            return kalos_string_allocate(state, "");    
        }
        if (fill == 0) {
            break;
        }
    }
done:
    buf[length] = 0;
    return kalos_string_commit_length(state, s, length);
}

kalos_string kalos_file_readline(kalos_state* state, kalos_object_ref* object) {
    return kalos_file_read_to_delim(state, object, '\n');
}

kalos_string kalos_file_read_all(kalos_state* state, kalos_object_ref* object) {
    return kalos_file_read_to_delim(state, object, '\0');
}

kalos_string kalos_file_read(kalos_state* state, kalos_object_ref* object, kalos_int size) {
    struct kalos_file* file = (*object)->context;
    if (file->fd == FILE_CLOSED) {
        kalos_value_error(state);
        return kalos_string_allocate(state, "");    
    }

    // Only fill the buffer for small reads that aren't totally satisfied
    if (size <= FILE_BUFFER_SIZE && size > file->buffer_size) {
        if (kalos_file_fill_buffer(file) == -1) {
            kalos_value_error(state);
            return kalos_string_allocate(state, "");    
        }
    }
    
    // Drain the ring buffer, then do a series of unbuffered reads for the rest
    kalos_writable_string s = kalos_string_allocate_writable_size(state, size);
    kalos_int original_size = size;
    char* buf = kalos_string_writable_c(state, s);
    int avail = min(size, min(file->buffer_size, FILE_BUFFER_SIZE - file->buffer_start));
    if (avail > 0) {
        memcpy(buf, file->ring_buffer + file->buffer_start, avail);
        buf += avail;
        size -= avail;
        file->buffer_start = (file->buffer_start + avail) % FILE_BUFFER_SIZE;
        file->buffer_size -= avail;
        avail = min(size, file->buffer_size);
        if (avail) {
            memcpy(buf, file->ring_buffer + file->buffer_start, avail);
            buf += avail;
            size -= avail;
            file->buffer_start = (file->buffer_start + avail) % FILE_BUFFER_SIZE;
            file->buffer_size -= avail;
        }
    }
    while (size) {
        int r = read(file->fd, buf, size);
        if (r == -1) {
            kalos_value_error(state);
            return kalos_string_allocate(state, "");    
        }
        if (r == 0) {
            break;
        }
        size -= r;
        buf += r;
    }
    kalos_string_writable_c(state, s)[original_size - size] = 0;
    return kalos_string_commit_length(state, s, original_size - size);
}

kalos_int kalos_file_write(kalos_state* state, kalos_object_ref* object, kalos_string* data) {
    struct kalos_file* file = (*object)->context;
    if (file->fd == FILE_CLOSED) {
        kalos_value_error(state);
        return 0;
    }
    int written = write(file->fd, kalos_string_c(state, *data), kalos_string_length(state, *data));
    if (written == -1) {
        kalos_value_error(state);
        return 0;
    }
    return written;
}

void kalos_file_close(kalos_state* state, kalos_object_ref* object) {
    struct kalos_file* file = (*object)->context;
    if (file->closeable) {
        close(file->fd);
    }
    file->fd = FILE_CLOSED;
}

kalos_object_ref kalos_file_get_stdin(kalos_state* state) {
    return kalos_file_open_fd(state, STDIN_FILENO, false);
}

kalos_object_ref kalos_file_get_stdout(kalos_state* state) {
    return kalos_file_open_fd(state, STDOUT_FILENO, false);
}

kalos_object_ref kalos_file_get_stderr(kalos_state* state) {
    return kalos_file_open_fd(state, STDERR_FILENO, false);
}

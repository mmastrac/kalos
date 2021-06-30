#include "kalos.h"
#include "_kalos_defines.h"

typedef struct {
    size_t size;
    kalos_mem_realloc_fn realloc;
    kalos_mem_free_fn free;
} kalos_buffer_header;

kalos_buffer kalos_buffer_alloc(kalos_state* state, size_t size) {
    kalos_buffer buffer;
    buffer.buffer = state->alloc(size + sizeof(kalos_buffer_header));
    kalos_buffer_header* header = (kalos_buffer_header*)buffer.buffer;
    header->realloc = state->realloc;
    header->free = state->free;
    header->size = size;
    buffer.buffer = PTR_BYTE_OFFSET(buffer.buffer, sizeof(kalos_buffer_header));
    return buffer;
}

size_t kalos_buffer_size(kalos_buffer buffer) {
    kalos_buffer_header* header = PTR_BYTE_OFFSET_NEG(buffer.buffer, sizeof(kalos_buffer_header));
    return header->size;
}

void kalos_buffer_resize(kalos_buffer* buffer, size_t size) {
    kalos_buffer_header* header = PTR_BYTE_OFFSET_NEG(buffer->buffer, sizeof(kalos_buffer_header));
    header = header->realloc(header, size + sizeof(kalos_buffer_header));
    header->size = size;
    buffer->buffer = PTR_BYTE_OFFSET(header, sizeof(kalos_buffer_header));
}

void kalos_buffer_free(kalos_buffer buffer) {
    if (buffer.buffer) {
        kalos_buffer_header* header = PTR_BYTE_OFFSET_NEG(buffer.buffer, sizeof(kalos_buffer_header));
        header->free(header);
    }
}

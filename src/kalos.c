#include "kalos.h"
#include "kalos_util.h"

typedef struct {
    size_t size;
    kalos_basic_environment* env;
} kalos_buffer_header;

kalos_buffer kalos_buffer_alloc(kalos_basic_environment* env, size_t size) {
    kalos_buffer buffer;
    buffer.buffer = env->alloc(size + sizeof(kalos_buffer_header));
    kalos_buffer_header* header = (kalos_buffer_header*)buffer.buffer;
    header->env = env;
    header->size = size;
    buffer.buffer = PTR_BYTE_OFFSET(buffer.buffer, sizeof(kalos_buffer_header));
    return buffer;
}

size_t kalos_buffer_size(kalos_buffer buffer) {
    kalos_buffer_header* header = PTR_BYTE_OFFSET_NEG(buffer.buffer, sizeof(kalos_buffer_header));
    return header->size;
}

void kalos_buffer_free(kalos_buffer buffer) {
    if (buffer.buffer) {
        kalos_buffer_header* header = PTR_BYTE_OFFSET_NEG(buffer.buffer, sizeof(kalos_buffer_header));
        header->env->free(header);
    }
}

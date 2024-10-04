#include "Buffer.h"
#include <stdlib.h>
#include <string.h> 

namespace conn {
#define DEFAULT_CAPCITY 1024 * 32

Buffer::Buffer():
    m_buf(NULL){

}

Buffer::~Buffer() {
    if (m_buf != NULL) {
        if (m_buf->data != NULL) {
            free(m_buf->data);
        }
        free(m_buf);
    }
}

int Buffer::append(void *buf, size_t bytes) {
    if (!buf) {
        return -1;
    }

    if (m_buf == NULL) {
        m_buf = (buffer_t)calloc(sizeof(buffer_st), 1);
        if (!m_buf) {
            return -1; 
        }
        m_buf->data = (char *)malloc(DEFAULT_CAPCITY);
        if (m_buf->data == NULL) {
            return -1;
        }
        m_buf->cap = DEFAULT_CAPCITY;
    }

    if (writeable_size() + bytes >= m_buf->cap) {
        char *buf = (char *)realloc(m_buf->data, m_buf->cap * 2);
        if (buf == NULL) {
            return -1;
        }
        m_buf->data = buf;
        m_buf->cap *= 2;
        memset(m_buf->data + m_buf->write_index, 0, m_buf->cap - m_buf->write_index);
    }
    memcpy(m_buf->data + m_buf->write_index, buf, bytes);
    m_buf->write_index += bytes;
    return 0;
}

int Buffer::readable_size() {
    return m_buf->write_index - m_buf->read_index;
}

int Buffer::writeable_size() {
    return m_buf->cap - m_buf->write_index;
}

char *Buffer::view_data() {
    if (m_buf->read_index == m_buf->write_index) {
        return NULL;
    }
    return m_buf->data + m_buf->read_index;
}

void Buffer::remove(size_t len) {
    m_buf->read_index += len;
    if (m_buf->read_index == m_buf->write_index) {
        m_buf->read_index = 0;
        m_buf->write_index = 0;
    }
}

}
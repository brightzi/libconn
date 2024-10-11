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

int Buffer::setdata(void *buf, size_t bytes) {
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
        m_buf->read_index = 0;
        m_buf->write_index = 0;
    }

    if (writeable_size() <= bytes) {
        size_t new_cap = m_buf->cap + bytes;
        char *buf = (char *)realloc(m_buf->data, new_cap);
        if (buf == NULL) {
            return -1;
        }
        m_buf->data = buf;
        m_buf->cap = new_cap;
        memset(m_buf->data + m_buf->write_index, 0, m_buf->cap - m_buf->write_index);
    }
    m_buf->write_index = 0;
    memset(m_buf->data, 0, m_buf->cap);
    memcpy(m_buf->data + m_buf->write_index, buf, bytes);
    m_buf->write_index += bytes;
    return 0;
}

int Buffer::size() {
    return m_buf->write_index - m_buf->read_index;
}

int Buffer::writeable_size() {
    return m_buf->cap - m_buf->write_index;
}

void *Buffer::data() {
    return m_buf->data;
}

}
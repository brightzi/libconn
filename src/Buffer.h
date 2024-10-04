#ifndef BUFFER_H
#define BUFFER_H
#include <stddef.h>

namespace conn {

typedef struct buffer_st {
    char *data;
    size_t write_index;
    size_t read_index;
    size_t cap;
}buffer_st, *buffer_t;

class Buffer {
public:
    Buffer();
    ~Buffer();

    char *view_data();
    void remove(size_t len);
    int append(void *buf, size_t bytes);
    int readable_size();
    int writeable_size();
private:
    buffer_t m_buf;
};


}
#endif
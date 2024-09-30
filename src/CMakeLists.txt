# 指定源文件
set(SOURCES 
    event_loop.c
    event.c
    ./dispatcher/select.c
    io.c
)

# 创建静态库
add_library(${CONN_LIB} STATIC ${SOURCES})
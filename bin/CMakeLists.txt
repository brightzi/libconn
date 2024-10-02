#add_executable(test_event test_event.c)
#add_executable(test_heap test_heap.c)

# 查找当前目录下所有的 .c 文件
file(GLOB SOURCES "*.c" "*.cpp")

# 循环创建可执行文件
foreach(SOURCE ${SOURCES})
    # 获取文件名（不带路径和扩展名）
    get_filename_component(EXEC_NAME ${SOURCE} NAME_WE)
    add_executable(${EXEC_NAME} ${SOURCE})
    target_link_libraries(${EXEC_NAME} ${CONN_LIB})
endforeach()
set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils.cpp)
add_executable(UTILS_TEST test.cpp ${HEADERR} ${SOURCE})
target_link_libraries(UTILS_TEST SP)
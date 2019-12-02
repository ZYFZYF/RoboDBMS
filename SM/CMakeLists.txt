set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/SM_Manager.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/SM_Manager.cpp)
message(STATUS ${HEADER})
message(STATUS ${SOURCE})
add_executable(SM_TEST test.cpp ${HEADER} ${SOURCE})
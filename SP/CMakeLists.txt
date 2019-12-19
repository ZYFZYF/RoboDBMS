set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Constant.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Handle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Manager.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Handle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Manager.cpp)
add_library(SP ${HEADER} ${SOURCE})
add_executable(SP_TEST_1 test1.cpp ${HEADERR} ${SOURCE})
add_executable(SP_TEST_2 test2.cpp ${HEADERR} ${SOURCE})
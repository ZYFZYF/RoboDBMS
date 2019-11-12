set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/def.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Handle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Manager.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Handle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SP_Manager.cpp)
message(STATUS ${HEADER})
message(STATUS ${SOURCE})
add_library(SP ${HEADER} ${SOURCE})
add_executable(SP_TEST test.cpp ${HEADERR} ${SOURCE})
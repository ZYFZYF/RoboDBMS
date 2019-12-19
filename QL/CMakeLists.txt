set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/QL_Manager.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/QL_Manager.cpp)
add_library(QL ${HEADER} ${SOURCE})

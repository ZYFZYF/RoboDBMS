set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/QL_Manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/QL_MultiTable.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/QL_Manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/QL_MultiTable.cpp)
add_library(QL ${HEADER} ${SOURCE})

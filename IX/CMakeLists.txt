set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/def.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_Manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_IndexHandle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_IndexScan.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_Manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_IndexHandle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_IndexScan.cpp)
message(STATUS ${HEADER})
message(STATUS ${SOURCE})
add_executable(IX_TEST test.cpp ${HEADER} ${SOURCE})


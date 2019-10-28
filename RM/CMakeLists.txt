set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_FileHandle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_FileScan.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_Manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_RID.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_Record.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_FileHandle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_FileScan.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_Manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_RID.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_Record.cpp)
message(STATUS ${HEADER})
message(STATUS ${SOURCE})
add_executable(RM_TEST test.cpp ${HEADER} ${SOURCE})
target_link_libraries(RM_TEST PF)
set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_Constant.h
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_BufferManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_FileHandle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_HashTable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_Manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_PageHandle.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_BufferManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_FileHandle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_HashTable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_Manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PF_PageHandle.cpp)
add_library(PF ${HEADER} ${SOURCE})
add_executable(PF_TEST_1 test1.cpp ${HEADER} ${SOURCE})
add_executable(PF_TEST_2 test2.cpp ${HEADER} ${SOURCE})
add_executable(PF_TEST_3 test3.cpp ${HEADER} ${SOURCE})
add_executable(PF_TEST_4 test4.cpp ${HEADER} ${SOURCE})
target_link_libraries(PF_TEST_1 SP)
target_link_libraries(PF_TEST_2 SP)
target_link_libraries(PF_TEST_3 SP)
target_link_libraries(PF_TEST_4 SP)
set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/SM_Constant.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SM_Manager.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/SM_Manager.cpp)
add_library(SM ${HEADER} ${SOURCE})
add_executable(SM_TEST test.cpp ${HEADER} ${SOURCE})
target_link_libraries(SM_TEST SP IX RM PF)
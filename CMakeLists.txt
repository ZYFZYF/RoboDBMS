cmake_minimum_required(VERSION 3.15)
project(RoboDBMS)
set(HEADER
        ${HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/Constant.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Attr.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/LinkList.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/PrintError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Statistics.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Utils.h)
set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/LinkList.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/PrintError.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Statistics.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Utils.cpp)
add_subdirectory(SP)
add_subdirectory(PF)
add_subdirectory(RM)
add_subdirectory(IX)
add_subdirectory(SM)
add_subdirectory(QL)
add_subdirectory(PS)
add_executable(MAIN main.cpp)
target_link_libraries(MAIN IX PF PS RM SM SP QL)

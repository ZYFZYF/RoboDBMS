find_package(FLEX)
find_package(BISON)
message(${PROJECT_BINARY_DIR})
bison_target(parser bison.y ${CMAKE_CURRENT_BINARY_DIR}/bison.cpp DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/bison.h)
flex_target(lexer flex.l ${CMAKE_CURRENT_BINARY_DIR}/flex.cpp COMPILE_FLAGS --noyywrap)
add_flex_bison_dependency(lexer parser)
add_executable(ttest ${CMAKE_CURRENT_BINARY_DIR}/flex.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(MAIN_SRC ${CMAKE_CURRENT_BINARY_DIR}/flex.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/bison.cpp)
add_executable(PS_TEST ${MAIN_SRC})
find_library(LEX_LIB l)
target_link_libraries(ttest ${LEX_LIB})


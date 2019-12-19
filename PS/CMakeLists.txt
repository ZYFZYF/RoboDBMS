find_package(FLEX)
find_package(BISON)
bison_target(parser bison.y ${PROJECT_BINARY_DIR}/bison.cpp DEFINES_FILE ${PROJECT_BINARY_DIR}/bison.h)
flex_target(lexer flex.l ${PROJECT_BINARY_DIR}/flex.cpp COMPILE_FLAGS --noyywrap)
add_flex_bison_dependency(lexer parser)

include_directories(${PROJECT_BINARY_DIR})
set(MAIN_SRC ${PROJECT_BINARY_DIR}/flex.cpp ${PROJECT_BINARY_DIR}/bison.cpp)
add_library(PS ${MAIN_SRC})


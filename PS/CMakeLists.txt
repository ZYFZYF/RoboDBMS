find_package(FLEX)
find_package(BISON)
message(${PROJECT_BINARY_DIR})
bison_target(parser bison.y ${PROJECT_BINARY_DIR}/bison.cpp DEFINES_FILE ${PROJECT_BINARY_DIR}/bison.h)
flex_target(lexer flex.l ${PROJECT_BINARY_DIR}/flex.cpp COMPILE_FLAGS --noyywrap)
add_flex_bison_dependency(lexer parser)
add_executable(ttest ${PROJECT_BINARY_DIR}/flex.cpp)

include_directories(${PROJECT_BINARY_DIR})
set(MAIN_SRC ${PROJECT_BINARY_DIR}/flex.cpp
        ${PROJECT_BINARY_DIR}/bison.cpp)
add_executable(PS_TEST ${MAIN_SRC})
find_library(LEX_LIB l)
target_link_libraries(ttest ${LEX_LIB})


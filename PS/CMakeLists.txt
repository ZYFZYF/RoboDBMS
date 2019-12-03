find_package(FLEX)
find_package(BISON)
message(${PROJECT_BINARY_DIR})
#flex_target(ttest_lexer flex.l ${PROJECT_BINARY_DIR}/test.cpp)
#add_executable(ttest ${CMAKE_CURRENT_BINARY_DIR}/test.cpp)
#find_library(LEX_LIB l)
#target_link_libraries(ttest ${LEX_LIB})


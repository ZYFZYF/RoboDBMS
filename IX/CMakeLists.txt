message(STATUS ${HEADER})
message(STATUS ${SOURCE})
add_executable(IX_TEST test.cpp ${HEADER} ${SOURCE})


list(APPEND SOURCES
    bloom_filter.cpp)

list(APPEND TEST_SOURCES
    bitvector_test.cpp
    bloom_filter_test.cpp)

add_library(estimlib STATIC ${SOURCES})
add_executable(unittests ${TEST_SOURCES})
target_link_libraries(unittests catch2 estimlib)

list(APPEND INCLUDES ${CMAKE_SOURCE_DIR}/src)
list(APPEND INCLUDES ${CMAKE_SOURCE_DIR}/src/third_party)

target_include_directories(estimlib PUBLIC ${INCLUDES})
target_include_directories(unittests PUBLIC ${INCLUDES})

add_test(NAME unittests
         COMMAND unittests)

set_property(TARGET estimlib PROPERTY CXX_STANDARD 20)
set_property(TARGET unittests PROPERTY CXX_STANDARD 20)

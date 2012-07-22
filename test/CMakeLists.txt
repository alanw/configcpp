set(CTEST_OUTPUT_ON_FAILURE TRUE)

find_package (GTest REQUIRED)

include_directories(
	include
    ${GTEST_INCLUDE_DIRS}
)

file(GLOB test_SOURCES
    *.cc
)

add_executable(test_configcpp
    ${test_SOURCES}
)

target_link_libraries(test_configcpp
    ${GTEST_BOTH_LIBRARIES}
    configcpp
)

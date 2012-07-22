file(GLOB example_SOURCES
    *.cc
)

add_executable(example_configcpp
    ${example_SOURCES}
)

target_link_libraries(example_configcpp
    configcpp
)

cmake_minimum_required(VERSION 2.6)

project (configcpp)
set(config_VERSION_MAJOR 0)
set(config_VERSION_MINOR 6)
set(config_VERSION_PATCH 0)
set(config_VERSION "${config_VERSION_MAJOR}.${config_VERSION_MINOR}.${config_VERSION_PATCH}")

option(BUILD_SHARED_LIBS "Build shared lib" ON)
option(BUILD_STATIC_LIBS "Build static lib" ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

find_package(Boost REQUIRED COMPONENTS system filesystem)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

include_directories(
    include
    ${Boost_INCLUDE_DIRS}
)

file(GLOB config_SOURCES
    src/*.cc
    src/detail/*.cc
)

if(BUILD_SHARED_LIBS)
    add_library(configcpp SHARED
        ${config_SOURCES}
    )

    target_link_libraries(configcpp
        ${Boost_LIBRARIES}
    )

    set_target_properties(configcpp PROPERTIES
        VERSION ${config_VERSION}
        SOVERSION ${config_VERSION_MAJOR}
    )

    install(TARGETS configcpp
        LIBRARY DESTINATION lib
    )
endif()

if(BUILD_STATIC_LIBS)
    add_library(configcpp_static STATIC
        ${config_SOURCES}
    )

    set_target_properties(configcpp_static PROPERTIES
        OUTPUT_NAME configcpp
    )

    install(TARGETS configcpp_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

file(GLOB config_HEADERS
    include/configcpp/*.h
)

install(FILES ${config_HEADERS}
    DESTINATION include/configcpp/
)

enable_testing()
add_subdirectory(test)
add_subdirectory(example)

message("** Build Summary **")
message("  Version:            ${config_VERSION}")
message("  Prefix:             ${CMAKE_INSTALL_PREFIX}")
message("  Build Type:         ${CMAKE_BUILD_TYPE}")
message("  Architecture:       ${CMAKE_SYSTEM_PROCESSOR}")

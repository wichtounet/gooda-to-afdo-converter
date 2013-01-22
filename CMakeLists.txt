cmake_minimum_required(VERSION 2.8.8)

project("gooda-to-afdo-converter")

#Set the version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

# Activate warnings and C++11 support
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(COMMON_CXX_FLAGS          "-pipe -pthread -std=c++11")
    set(CMAKE_CXX_FLAGS           "${COMMON_CXX_FLAGS} -g -pedantic -Wunused -Wlogical-op -Wuninitialized -Wall -Wextra -Wno-long-long -Winit-self")
    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE   "${COMMON_CXX_FLAGS} -DNDEBUG -O3 -fomit-frame-pointer -march=native")
else()
    MESSAGE(FATAL_ERROR "ERROR: The converter can only be compiled on GCC (CLang produces buggy executable)")
endif()

# Add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
        WORKING_DIRECTORY doc
        COMMENT "Generating API documentation with
        Doxygen" VERBATIM
        )
endif(DOXYGEN_FOUND)

# The executable should go to the bin directory
set(EXECUTABLE_OUTPUT_PATH bin)

# All the headers are in the include directory
include_directories(include)

# Create the executable

file(
    GLOB_RECURSE
    converter_files
    src/*.cpp
)

file(GLOB to_remove src/main.cpp)
list(REMOVE_ITEM converter_files ${to_remove})

add_library(Converter OBJECT ${converter_files})

# Create the converter executable

add_executable(converter $<TARGET_OBJECTS:Converter> src/main.cpp)

TARGET_LINK_LIBRARIES(converter boost_program_options)

# Specifications for the installation

INSTALL(TARGETS converter
    RUNTIME DESTINATION bin
)

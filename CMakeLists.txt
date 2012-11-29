cmake_minimum_required(VERSION 2.8.8)

project("gooda-to-afdo-converter")

#Set the version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

# Activate warnings and C++11 support
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(COMMON_CXX_FLAGS          "-pipe -pthread -std=c++11")
    set(CMAKE_CXX_FLAGS           "${COMMON_CXX_FLAGS} -g -pedantic -Wuninitialized -Wall -Wextra -Wno-long-long -Winit-self -fstack-protector")
    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE   "${COMMON_CXX_FLAGS} -DNDEBUG -O3 -fomit-frame-pointer -march=native")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(COMMON_CXX_FLAGS          "-std=c++11 -Qunused-arguments")
    set(CMAKE_CXX_FLAGS           "${COMMON_CXX_FLAGS} -g -pedantic -Wuninitialized -Wall -Wextra -Wno-long-long -Winit-self")
    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE   "${COMMON_CXX_FLAGS} -DLOGGING_DISABLE -DNDEBUG -O3 -fomit-frame-pointer -march=native")
else()
    MESSAGE(FATAL_ERROR "ERROR: eddic can only be compiled on gcc and clang for now")
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
	source_files
	src/*.cpp
)

add_executable(converter ${source_files})

TARGET_LINK_LIBRARIES(converter boost_program_options)
#TARGET_LINK_LIBRARIES(converter gcov)

# Specifications for the installation

INSTALL(TARGETS converter
    RUNTIME DESTINATION bin
)

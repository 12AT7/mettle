cmake_minimum_required(VERSION 3.2)
project(Mettle LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS program_options filesystem iostreams)
find_package(Threads REQUIRED)

# Download bencode.hpp, but keep it in the binary directory so both Git and
# CMake realize it is a transient artifact.
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/bencode.hpp"
	COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/scripts/vendorize_bencode.py
	COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/include/bencode.hpp ${CMAKE_CURRENT_BINARY_DIR}/include
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/vendorize_bencode.py
	COMMENT "Download bencode.hpp"
	VERBATIM
	)
add_custom_target(bencode DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/include/bencode.hpp")

file(GLOB LIBRARY_SRC
    "src/libmettle/*.cpp"
    "src/libmettle/log/*.cpp"
    )

if (UNIX)
    file(GLOB PLATFORM_SRC
    	"src/libmettle/posix/*.cpp"
    	"src/posix/*.cpp"
    	"src/libmettle/log/posix/*.cpp"
	)
elseif (WIN32)
    message(FATAL_ERROR "Mettle CMakeLists.txt not tested on Windows (although Mettle itself is")
    file(GLOB PLATFORM_SRC
    	"src/libmettle/windows/*.cpp"
    	"src/windows/*.cpp"
    	"src/libmettle/log/windows/*.cpp"
	)
else (UNIX)
    message(FATAL_ERROR "Mettle CMakeLists.txt supported only on Posix for now")
endif (UNIX)

add_library(libmettle SHARED ${LIBRARY_SRC} ${PLATFORM_SRC})
add_dependencies(libmettle bencode)
target_include_directories(libmettle
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(libmettle
    PUBLIC
        Boost::program_options Boost::iostreams Threads::Threads
)

file(GLOB EXECUTABLE_SRC
	"src/mettle/*.cpp"
	"src/mettle/posix/*.cpp"
	)

add_executable(mettle ${EXECUTABLE_SRC})
add_dependencies(mettle bencode)

target_include_directories(mettle
    PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(mettle
	libmettle Boost::program_options Boost::iostreams Threads::Threads)

if (BUILD_TESTING)
    add_subdirectory(test)
endif (BUILD_TESTING)

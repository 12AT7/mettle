cmake_minimum_required(VERSION 3.12)
project(mettle VERSION 0.2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS program_options filesystem iostreams)
find_package(Threads REQUIRED)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Download bencode.hpp, but keep it in the binary directory so both Git and
# CMake realize it is a transient artifact.
# add_custom_command(
# 	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/bencode.hpp"
# 	COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/scripts/vendorize_bencode.py
# 	COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/include/bencode.hpp ${CMAKE_CURRENT_BINARY_DIR}/generated
# 	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/vendorize_bencode.py
# 	COMMENT "Download bencode.hpp"
# 	VERBATIM
# 	)
# add_custom_target(bencode DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generated/bencode.hpp")
add_compile_definitions("METTLE_VERSION=0.2")

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
    message(FATAL_ERROR "Mettle CMakeLists.txt not tested on Windows (although Mettle itself is)")
    file(GLOB PLATFORM_SRC
    	"src/libmettle/windows/*.cpp"
    	"src/windows/*.cpp"
    	"src/libmettle/log/windows/*.cpp"
	)
else (UNIX)
    message(FATAL_ERROR "Mettle CMakeLists.txt supported only on Posix for now")
endif (UNIX)

add_library(libmettle SHARED ${LIBRARY_SRC} ${PLATFORM_SRC})
# add_dependencies(libmettle bencode)
target_include_directories(libmettle
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(libmettle
    PUBLIC
        Boost::program_options Boost::iostreams Threads::Threads
)
set_target_properties(libmettle PROPERTIES OUTPUT_NAME "mettle")

file(GLOB EXECUTABLE_SRC
	"src/mettle/*.cpp"
	"src/mettle/posix/*.cpp"
	)

add_executable(mettle ${EXECUTABLE_SRC})
# add_dependencies(mettle bencode)
add_dependencies(mettle)

target_include_directories(mettle
    PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(mettle
	libmettle Boost::program_options Boost::iostreams Threads::Threads)

if (BUILD_TESTING)
    add_subdirectory(test)
endif (BUILD_TESTING)

include(GNUInstallDirs)
install(TARGETS libmettle mettle
	EXPORT MettleTargets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)
install(
	DIRECTORY "${CMAKE_SOURCE_DIR}/include" 
	DESTINATION ${CMAKE_INSTALL_PREFIX}
	)
install(EXPORT MettleTargets
	FILE MettleConfig.cmake
	NAMESPACE mettle::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mettle
	)

# CMake sucks.  Nonetheless, this guy figured out some useful stuff: 
# https://unclejimbo.github.io/2018/06/08/Modern-CMake-for-Library-Developers/
include(CMakePackageConfigHelpers)
configure_package_config_file(
	${CMAKE_SOURCE_DIR}/cmake/MettleConfig.cmake.in
	${CMAKE_BINARY_DIR}/cmake/MettleConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mettle
)
write_basic_package_version_file(
	${CMAKE_BINARY_DIR}/cmake/MettleConfigVersion.cmake
	VERSION ${METTLE_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(
    FILES
	${CMAKE_BINARY_DIR}/cmake/MettleConfig.cmake
	${CMAKE_BINARY_DIR}/cmake/MettleConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mettle
)


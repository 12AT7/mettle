file(GLOB_RECURSE TEST_SRC *.cpp)
foreach(TEST ${TEST_SRC})
	get_filename_component(TEST_TARGET ${TEST} NAME_WE)
	add_executable(${TEST_TARGET} 
		${TEST} 
		${CMAKE_CURRENT_SOURCE_DIR}/../src/mettle/posix/run_test_file.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/../src/mettle/test_file.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/../src/mettle/run_test_files.cpp
		)
	target_link_libraries(${TEST_TARGET} libmettle)
	add_dependencies(${TEST_TARGET} bencode)
	target_include_directories(${TEST_TARGET}
		PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/../include
		)
	add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

endforeach()

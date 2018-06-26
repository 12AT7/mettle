file(GLOB_RECURSE TEST_SRC *.cpp)
foreach(TEST ${TEST_SRC})
	get_filename_component(TEST_TARGET ${TEST} NAME_WE)
	add_executable(${TEST_TARGET} 
		${TEST} 
		${CMAKE_SOURCE_DIR}/src/mettle/posix/run_test_file.cpp
		${CMAKE_SOURCE_DIR}/src/mettle/test_file.cpp
		${CMAKE_SOURCE_DIR}/src/mettle/run_test_files.cpp
		)
	target_link_libraries(${TEST_TARGET} mettle)
endforeach()

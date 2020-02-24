include(CheckIPOSupported)

include_directories("../../include")
file(GLOB_RECURSE SOURCE_LIST "./*.cpp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../bin")

add_executable(ShitVM ${SOURCE_LIST})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	check_ipo_supported(RESULT isIPOSupported)
	if(isIPOSupported)
		set_property(TARGET ShitVM PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

install(TARGETS ShitVM DESTINATION "bin")
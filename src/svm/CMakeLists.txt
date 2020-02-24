include_directories("../../include")
file(GLOB_RECURSE SOURCE_LIST "./*.cpp")

set_property(PROPERTY RUNTIME_OUTPUT_DIRECTORY "../../bin")
link_libraries(ShitCore)
add_executable(ShitVM ${SOURCE_LIST})

install(TARGETS ShitVM DESTINATION "bin")
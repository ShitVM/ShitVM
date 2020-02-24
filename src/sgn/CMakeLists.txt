include_directories("../../include")
file(GLOB_RECURSE SOURCE_LIST "./*.cpp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../../bin")

add_library(ShitGen STATIC ${SOURCE_LIST})

install(DIRECTORY "../../include/sgn" DESTINATION "include")
install(TARGETS ShitGen DESTINATION "lib")
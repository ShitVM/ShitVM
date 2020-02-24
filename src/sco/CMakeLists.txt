include_directories("../../include")
file(GLOB_RECURSE SOURCE_LIST "./*.cpp")

set_property(PROPERTY RUNTIME_OUTPUT_DIRECTORY "../../lib")
add_library(ShitCore STATIC ${SOURCE_LIST})

install(DIRECTORY "../../include/sco" DESTINATION "include")
install(TARGETS ShitGen DESTINATION "lib")
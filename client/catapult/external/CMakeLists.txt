project(external)

file(GLOB _sources LIST_DIRECTORIES false CONFIGURE_DEPENDS "donna/*.c" "donna/*.h")

SOURCE_GROUP("donna" FILES ${_sources})

add_library(external 
	${_sources}
	README.txt
)

target_compile_definitions(external PUBLIC NO_MISALIGNED_ACCESSES) #sha3
target_link_libraries(external build.defaults)
catapult_add_openssl_dependencies(external)

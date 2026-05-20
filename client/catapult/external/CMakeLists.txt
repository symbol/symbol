project(external)

file(GLOB external_donna_SRC
	"donna/*.c"
	"donna/*.h"
)

SOURCE_GROUP("donna" FILES ${external_donna_SRC})

add_library(external
	${external_donna_SRC}
	README.txt
)

target_compile_definitions(external PUBLIC NO_MISALIGNED_ACCESSES) #sha3
target_link_libraries(external build.defaults)
catapult_add_openssl_dependencies(external)

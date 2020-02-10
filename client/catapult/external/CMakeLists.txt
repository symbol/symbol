cmake_minimum_required(VERSION 3.14)
project(external)
add_definitions(-DNO_MISALIGNED_ACCESSES) # sha3

file(GLOB external_donna_SRC
	"donna/*.c"
	"donna/*.h"
)
file(GLOB external_sha256_SRC
	"sha256/*.c"
	"sha256/*.h"
)
file(GLOB external_sha3_SRC
	"sha3/*.c"
	"sha3/*.h"
)
file(GLOB external_ripemd160_SRC
	"ripemd160/*.c"
	"ripemd160/*.h"
)
file(GLOB external_tiny_aes_SRC
	"tiny-aes-c/*.c"
	"tiny-aes-c/*.h"
	"tiny-aes-c/*.hpp"
)

SOURCE_GROUP("donna" FILES ${external_donna_SRC})
SOURCE_GROUP("sha256" FILES ${external_sha256_SRC})
SOURCE_GROUP("sha3" FILES ${external_sha3_SRC})
SOURCE_GROUP("ripemd160" FILES ${external_ripemd160_SRC})
SOURCE_GROUP("tinyaes" FILES ${external_tiny_aes_SRC})

add_library(external
	${external_donna_SRC}
	${external_sha256_SRC}
	${external_sha3_SRC}
	${external_ripemd160_SRC}
	${external_tiny_aes_SRC}
	README.txt)

catapult_add_ssl_dependencies(external)

set_property(TARGET external PROPERTY POSITION_INDEPENDENT_CODE ON)

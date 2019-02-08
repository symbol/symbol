cmake_minimum_required(VERSION 3.2)
project(external)

file(GLOB external_ref10_SRC
	"ref10/*.cpp"
	"ref10/*.c"
	"ref10/*.h")
file(GLOB external_sha256_SRC
	"sha256/*.c"
	"sha256/*.h")
file(GLOB external_sha3_SRC
	"sha3/*.c"
	"sha3/*.h")
file(GLOB external_ripemd160_SRC
	"ripemd160/*.c"
	"ripemd160/*.h"
)

SOURCE_GROUP("ref10" FILES ${external_ref10_SRC})
SOURCE_GROUP("sha256" FILES ${external_sha256_SRC})
SOURCE_GROUP("sha3" FILES ${external_sha3_SRC})
SOURCE_GROUP("ripemd160" FILES ${external_ripemd160_SRC})

add_library(external ${external_ref10_SRC} ${external_sha256_SRC} ${external_sha3_SRC} ${external_ripemd160_SRC} README.txt)
set_property(TARGET external PROPERTY POSITION_INDEPENDENT_CODE ON)

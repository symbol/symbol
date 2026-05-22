project(external)
add_target(
	external LIBRARY
	DEFINITIONS NO_MISALIGNED_ACCESSES	#sha3
	LINK_LIBS OpenSSL::SSL
	SOURCES ./donna README.txt
)

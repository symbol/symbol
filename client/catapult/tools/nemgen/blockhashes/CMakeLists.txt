set(TARGET_NAME catapult.tools.nemgen.blockhashes)

add_target(${TARGET_NAME} LIBRARY
	LINK_LIBS catapult.chain
	SOURCES ./
)

# tool has chain dependency so it must be able to access src

add_target(
	catapult.thread LIBRARY
	LINK_LIBS
		$<TARGET_NAME_IF_EXISTS:catapult.asio>
		catapult.model
		catapult.utils
	SOURCES ./
)
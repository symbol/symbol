add_target(tests.catapult.zeromq TEST
	LINK_LIBS tests.catapult.test.core
	LABELS Integration
	SOURCES
		./
		./test
)
catapult_add_zeromq_dependencies(tests.catapult.zeromq)

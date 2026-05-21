add_target(
	tests.catapult.int.node TEST
	WNO_DERIVED_LIB
	LINK_LIBS
		tests.catapult.int.node.test
		tests.catapult.test.nemesis
	LABELS Integration
	SOURCES ./ 
)

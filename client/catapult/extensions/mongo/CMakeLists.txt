
add_subdirs()

add_target(
	mongo EXTENSION SHARED
	LINK_LIBS 
		${MongoDB_IMPORTED_TARGETS}
		catapult.mongo
	FOLDER "extensions"
	DEPENDENTS mongo
	SOURCES ./
)

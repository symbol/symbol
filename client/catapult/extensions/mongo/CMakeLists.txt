add_subdirs()
add_target(
	extension.mongo LIBRARY TYPE SHARED
	LINK_LIBS 
		${MongoDB_IMPORTED_TARGETS}
		catapult.mongo
	FOLDER "extensions"
	DEPENDENTS mongo
	SOURCES ./
)

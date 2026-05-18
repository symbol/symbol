add_target(
    tests.catapult.mongo.plugins.locksecret TEST
    NO_DERIVED_LIB
    INCLUDE_DIRS 
        "${CMAKE_SOURCE_DIR}/plugins/txes/lock_secret"
    LINK_LIBS
        ${MongoDB_IMPORTED_TARGETS}
    SOURCES 
        ./ 
        ./mappers
)

add_subdirs()

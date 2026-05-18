add_target(
    catapult.mongo.plugins.accountlink LIBRARY
    TYPE SHARED
    LINK_LIBS 
        ${MongoDB_IMPORTED_TARGETS}
        catapult.mongo
    DEPENDENTS mongo
    SOURCES ./
)

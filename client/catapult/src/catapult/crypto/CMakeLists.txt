if(EXISTS ${CMAKE_SOURCE_DIR}/external)
    add_target(
        catapult.crypto LIBRARY
        LINK_LIBS catapult.utils external OpenSSL::SSL
        SOURCES
    )
    target_include_directories(catapult.crypto SYSTEM BEFORE PUBLIC
        ${CMAKE_SOURCE_DIR}/external
    )
else()
    add_target(
        catapult.crypto LIBRARY
        LINK_LIBS catapult.utils OpenSSL::SSL
        SOURCES
    )
endif()

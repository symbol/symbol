add_target(
    catapult.version.nix LIBRARY
    TYPE OBJECT
    LINK_LIBS
    SOURCES ./
)
target_link_libraries(catapult.version.nix $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)

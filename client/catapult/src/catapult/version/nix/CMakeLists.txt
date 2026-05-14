add_target(
    catapult.version LIBRARY
    TYPE_OBJECT
    SOURCES
)
target_link_libraries(catapult.version.nix $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)

include_directories(../../../external)

catapult_test_executable_target(tests.catapult.crypto crypto NOLIB)
catapult_add_openssl_dependencies(tests.catapult.crypto)

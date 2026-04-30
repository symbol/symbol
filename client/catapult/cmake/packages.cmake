# All third-party dependencies are defined here. 

find_package(Boost 1.90.0 REQUIRED COMPONENTS atomic chrono date_time filesystem log log_setup program_options regex thread)
find_package(OpenSSL 3.6.1 REQUIRED)
find_package(RocksDB 10.6.2 REQUIRED)
set(RocksDB_IMPORTED_TARGETS $<IF:$<TARGET_EXISTS:RocksDB::rocksdb>,RocksDB::rocksdb,RocksDB::rocksdb-shared>)

find_package(mongocxx 4.1.4 REQUIRED)
find_package(mongoc 2.2.1 REQUIRED)
find_package(cppzmq 4.11.0 REQUIRED)

if(ENABLE_TESTS)
    find_package(GTest 1.16.0 REQUIRED)
    find_package(benchmark 1.9.4 REQUIRED)
    if(USE_VCPKG OR USE_CONAN)
        set(GTest_IMPORTED_TARGETS GTest::gtest)
    elseif(USE_METAL)
        set(GTest_IMPORTED_TARGETS GTEST_LIBRARIES)
    endif()
else()
    set(GTest_VERSION "n.a." CACHE STRING)
    set(benchmark_VERSION "n.a." CACHE STRING)
endif()

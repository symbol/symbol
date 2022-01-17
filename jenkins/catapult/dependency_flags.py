DEPENDENCY_FLAGS = {
	'facebook_rocksdb': [
		'-DPORTABLE=1',
		'-DWITH_TESTS=OFF',
		'-DWITH_TOOLS=OFF',
		'-DWITH_BENCHMARK_TOOLS=OFF',
		'-DWITH_CORE_TOOLS=OFF',
		'-DWITH_GFLAGS=OFF'
	],

	'google_googletest': ['-DCMAKE_POSITION_INDEPENDENT_CODE=ON'],
	'google_benchmark': ['-DBENCHMARK_ENABLE_GTEST_TESTS=OFF'],

	'mongodb_mongo-c-driver': [
		'-DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF',
		'-DENABLE_MONGODB_AWS_AUTH=OFF',
		'-DENABLE_TESTS=OFF',
		'-DENABLE_EXAMPLES=OFF',
		'-DENABLE_SASL=OFF'
	],
	'mongodb_mongo-cxx-driver': ['-DCMAKE_CXX_STANDARD=17'],

	'zeromq_libzmq': ['-DWITH_TLS=OFF'],
	'zeromq_cppzmq': ['-DCPPZMQ_BUILD_TESTS=OFF']
}

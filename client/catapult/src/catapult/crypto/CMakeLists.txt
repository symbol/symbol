cmake_minimum_required(VERSION 3.2)

include_directories(../../../external)

catapult_library_target(catapult.crypto)

message("--- locating openssl dependencies ---")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# https://gitlab.kitware.com/cmake/cmake/issues/16885
	include_directories(${OPENSSL_ROOT_DIR}/include)
	list(APPEND OPENSSL_LIBS ${OPENSSL_ROOT_DIR}/lib/libssl.dylib)
	list(APPEND OPENSSL_LIBS ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib)

	message("OpenSSL root: ${OPENSSL_ROOT_DIR}")
else()
	find_package(OpenSSL REQUIRED)
	list(APPEND OPENSSL_LIBS OpenSSL::SSL)

	message("OpenSSL Version: ${OPENSSL_VERSION} ${OPENSSL_INCLUDE_DIR} ${OPENSSL_LIBRARIES}")
endif()

message("OpenSSL libs: ${OPENSSL_LIBS}")

target_link_libraries(catapult.crypto catapult.utils external ${OPENSSL_LIBS})

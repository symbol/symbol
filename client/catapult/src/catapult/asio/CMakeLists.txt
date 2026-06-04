# Boost.Asio is compiled once into this dedicated shared library so that every module shares a single instance of
# Asio's runtime and thread-local state (see cmake/settings_all.cmake). This relies on ELF symbol interposition and
# is therefore gated to non-Windows targets: Windows (PE) cannot interpose Asio's header-only template TLS state, so
# Asio is kept header-only there and this library is not built.
if(NOT WIN32)
	add_target(catapult.asio LIBRARY TYPE SHARED
		LINK_LIBS OpenSSL::SSL
		SOURCES ./AsioImpl.cpp)

	# override file-level visibility to default so all Asio symbols are exported from the DSO and can be interposed
	set_source_files_properties(./AsioImpl.cpp PROPERTIES COMPILE_FLAGS "-fvisibility=default")
endif()

add_target(catapult.asio LIBRARY TYPE SHARED
	LINK_LIBS OpenSSL::SSL
	SOURCES ./AsioImpl.cpp)

# On GCC/Clang: override file-level visibility to default so all Asio symbols
# are exported from the DSO. Not needed on MSVC where BOOST_ASIO_DYN_LINK +
# BOOST_ASIO_SOURCE (auto-defined by src.hpp) generate __declspec(dllexport).
if(NOT MSVC)
	set_source_files_properties(./AsioImpl.cpp PROPERTIES COMPILE_FLAGS "-fvisibility=default")
endif()

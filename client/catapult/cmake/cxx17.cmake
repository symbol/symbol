set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE YES)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Sets the default logging level affecting printout of message() commands
if(NOT DEFINED CMAKE_MESSAGE_LOG_LEVEL)
	set(CMAKE_MESSAGE_LOG_LEVEL "WARNING" CACHE STRING "Default logging level")
endif()

if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
	set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Default architecture")
endif()

if(CMAKE_VERSION VERSION_LESS "4.0")
	cmake_policy(SET CMP0063 NEW) # Honor visibility properties for all target types (since CMake 3.13):
endif()

cmake_policy(SET CMP0074 NEW) # FindXXX packages will use <PackageName>_ROOT variables (since CMake 3.12):
cmake_policy(SET CMP0077 NEW) # Disallow option() to override normal variables (since CMake 3.13):

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP (since CMake 3.24):
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
	cmake_policy(SET CMP0135 NEW)
endif()
# Avoid warning about FindBoost module (since CMake 3.30):
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
	cmake_policy(SET CMP0167 NEW)
endif()

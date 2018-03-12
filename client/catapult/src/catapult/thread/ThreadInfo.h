#pragma once
#include <string>

namespace catapult { namespace thread {

	/// Gets the maximum supported thread name length (excluding NUL-terminator).
	size_t GetMaxThreadNameLength();

	/// Sets a thread \a name in a platform-dependent way.
	/// \note Depending on platform capabilities, the name might be truncated.
	void SetThreadName(const std::string& name);

	/// Gets a thread name in a platform-dependent way.
	std::string GetThreadName();
}}

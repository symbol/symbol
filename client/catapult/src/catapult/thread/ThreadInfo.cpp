/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "ThreadInfo.h"
#ifdef _WIN32
#include <array>
#include <cstring>
#else
#include <errno.h>
#include <pthread.h>
#endif
#include <string>
#include <tuple>

namespace catapult { namespace thread {
	namespace {
#if defined(PTHREAD_MAX_NAMELEN_NP)
		constexpr std::size_t kMaxThreadNameBytes = PTHREAD_MAX_NAMELEN_NP;
#elif defined(__APPLE__)
		constexpr std::size_t kMaxThreadNameBytes = 64; // Including the null terminator
#elif defined(__linux__) && HAVE_PTHREAD_SETNAME_NP
		constexpr std::size_t kMaxThreadNameBytes = 16; // Including the null terminator
#elif defined(_WIN32)
		constexpr std::size_t kMaxThreadNameBytes = 64; // Including the null terminator (arbitrary choice)
#define HAVE_PTHREAD_SETNAME_NP 1 // We provide our own implementation of pthread_setname_np on Windows
#define HAVE_PTHREAD_GETNAME_NP 1 // We provide our own implementation of pthread_getname_np on Windows
#elif defined(__GLIBC__)
		constexpr std::size_t kMaxThreadNameBytes = 16; // Including the null terminator
#else
		constexpr std::size_t kMaxThreadNameBytes = 0; // Disable thread naming on unsupported platforms
#endif

#ifdef _WIN32

		thread_local std::array<char, kMaxThreadNameBytes> t_threadName = { '\0' };

		constexpr int pthread_self() {
			return 0;
		}

		int pthread_setname_np(const char* name) {
			/*
			* We've already truncated the name to fit the maximum length in SetThreadName,
			* so we can safely copy it here without worrying about truncation.
			* We automatically include the NUL-terminator in the copy length.
			*/
			auto bytesCount = std::strlen(name) + 1;
			std::memcpy(t_threadName.data(), name, bytesCount);
			return 0;
		}

		int pthread_getname_np(int, char* name, size_t len) {
			if (!name || 0 == len)
				return 1;
			/*
			* From GetThreadName we already know `name` is a null terminated array of
			* at least kMaxThreadNameBytes characters. We can safely copy it the
			* whole content of t_threadName, which is also null terminated.
			*/
			std::memcpy(name, t_threadName.data(), kMaxThreadNameBytes);
			return 0;
		}

#endif
	}

	std::size_t GetMaxThreadNameLength() {
		return kMaxThreadNameBytes ? kMaxThreadNameBytes - 1 : 0;
	}

	void SetThreadName(const std::string& name) {
		if (name.empty())
			return;

		const std::size_t maxLength = GetMaxThreadNameLength();
		if (0 == maxLength)
			return;

		// We truncate from the front of the name to preserve any unique suffixes,
		// which are often more useful for debugging than a common prefix.
		auto startOffset = name.size() > maxLength ? name.size() - maxLength : 0;
		auto truncatedName = name.substr(startOffset, maxLength);

#if defined(HAVE_PTHREAD_SET_NAME_NP) && HAVE_PTHREAD_SET_NAME_NP
		std::ignore = pthread_set_name_np(::pthread_self(), truncatedName.data());
#elif defined(HAVE_PTHREAD_SETNAME_NP) && HAVE_PTHREAD_SETNAME_NP
#if defined(__APPLE__) || defined(_WIN32)
		std::ignore = pthread_setname_np(truncatedName.data());
#else
		std::ignore = pthread_setname_np(pthread_self(), truncatedName.c_str());
#endif
#endif
	}

	std::string GetThreadName() {
		std::string ret;
		if (0 == kMaxThreadNameBytes)
			return ret;

		char buffer[kMaxThreadNameBytes] = { '\0' };
		ret.reserve(kMaxThreadNameBytes);
#if defined(HAVE_PTHREAD_GETNAME_NP) && HAVE_PTHREAD_GETNAME_NP
		if (0 == pthread_getname_np(pthread_self(), buffer, sizeof(buffer))) {
			ret.append(buffer, buffer + std::strlen(buffer));
		}

#elif defined(HAVE_PTHREAD_GET_NAME_NP) && HAVE_PTHREAD_GET_NAME_NP
			pthread_get_name_np(pthread_self(), buffer, sizeof(buffer));
			ret.append(buffer, buffer + std::strlen(buffer));
#endif
			return ret;
		}
	}
}

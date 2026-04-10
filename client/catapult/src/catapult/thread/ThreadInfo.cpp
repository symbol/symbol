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

	/*
	* Windows and Apple platforms support thread names up to 64 characters,
	* while Linux (glibc) supports up to 16 characters
	* Note this value does not include the NUL-terminator.
	*/
#if defined(_WIN32) || defined(__APPLE__)
    constexpr std::size_t kMaxThreadNameLength = 63;
#else
    constexpr std::size_t kMaxThreadNameLength = 15;
#endif

	size_t GetMaxThreadNameLength() {
        return kMaxThreadNameLength;
	}

	namespace {
#ifdef _WIN32
		
		thread_local std::array<char, kMaxThreadNameLength + /*NUL-terminator*/ 1> t_threadName = {'\0'};

		int pthread_self() {
			return 0;
		}

		int pthread_setname_np(const char* name) {
			/*
			* We've already truncated the name to fit the maximum length in SetThreadName, 
			* so we can safely copy it here without worrying about truncation.
            * We automatically include the NUL-terminator in the copy length.
			*/
			auto copyLength = std::strlen(name) + 1;
			std::memcpy(t_threadName.data(), name, copyLength);
			return 0;
		}

		int pthread_getname_np(int, char* name, size_t len) {
			if (!name || 0 == len)
				return 1;
			/*
			* From GetThreadName we already know `name` is a null terminated array of 
			* at least kMaxThreadNameLength characters. We can safely copy it the
            * whole content of t_threadName, which is also null terminated.
			*/
            std::memcpy(name, t_threadName.data(), kMaxThreadNameLength);
			return 0;
		}
#elif !defined(__APPLE__)
		// add overload that sets name of current thread
		int pthread_setname_np(const char* name) {
			return ::pthread_setname_np(pthread_self(), name);
		}
#endif
	}

	void SetThreadName(const std::string& name) {
        if (name.empty())
			return;

		auto truncatedName = name.substr(0, GetMaxThreadNameLength());
		std::ignore = pthread_setname_np(truncatedName.c_str());
	}

	std::string GetThreadName() {
#if defined(__APPLE__) || defined(__GLIBC__) || defined(_WIN32)
        std::string ret(kMaxThreadNameLength, char(0)); // pre-allocate buffer for name retrieval
		std::ignore = pthread_getname_np(pthread_self(), &ret[0], ret.size());
		return std::string(ret.c_str());
#else
		// musl libc (from alpine) defines __GNU_SOURCE__ but it only has pthread_setname_np
		return std::string();
#endif
	}
}}

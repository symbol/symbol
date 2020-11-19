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
#include "catapult/utils/Logging.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <pthread.h>
#endif

namespace catapult { namespace thread {

	size_t GetMaxThreadNameLength() {
#ifdef _WIN32
		return 63; // arbitrary max
#elif defined(__APPLE__)
		return 63;
#else
		return 15;
#endif
	}

	namespace {
#ifdef _WIN32
		// add pthread shims using tls
		thread_local std::string t_threadName;

		int pthread_self() {
			return 0;
		}

		int pthread_setname_np(const char* name) {
			t_threadName = name;
			return 0;
		}

		int pthread_getname_np(int, char* name, size_t len) {
			std::memcpy(name, t_threadName.c_str(), len);
			name[len - 1] = '\0';
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
		auto truncatedName = name.substr(0, GetMaxThreadNameLength());
		pthread_setname_np(truncatedName.c_str());
	}

	std::string GetThreadName() {
#if defined(__APPLE__) || defined(__GLIBC__) || defined(_WIN32)
		std::string name;
		name.resize(GetMaxThreadNameLength() + 1); // include space for NUL-terminator
		pthread_getname_np(pthread_self(), &name[0], name.size());
		return name.substr(0, name.find_first_of('\0'));
#else
		// musl libc (from alpine) defines __GNU_SOURCE__ but it only has pthread_setname_np
		return std::string();
#endif
	}
}}

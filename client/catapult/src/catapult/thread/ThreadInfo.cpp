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
#include <cstring>
#include <tuple>
#if defined(__APPLE__) || defined(__GLIBC__)
#include <pthread.h>
#endif

namespace catapult { namespace thread {

	namespace {
#if defined(__APPLE__)

		constexpr std::size_t kMaxThreadNameBytes = 64; // Including the null terminator

		int pthread_setname_np(pthread_t, const char* name) {
			/*
			* On macOS, pthread_setname_np only accepts the thread name without the thread ID,
			* and it applies to the current thread. Therefore, we can ignore the thread ID
			* and directly set the thread name for the current thread.
			*/
			return ::pthread_setname_np(name);
		}

#elif defined(__GLIBC__)

		constexpr std::size_t kMaxThreadNameBytes = 16; // Including the null terminator

#elif defined(_WIN32)

		constexpr std::size_t kMaxThreadNameBytes = 64; // Including the null terminator (arbitrary choice)

		thread_local char storage[kMaxThreadNameBytes] = { '\0' };

		constexpr int pthread_self() {
			return 0;
		}

		int pthread_setname_np(int, const char* name) {
			/*
			* We've already truncated the name to fit the maximum length in SetThreadName,
			* so we can safely copy it here without worrying about truncation.
			* We automatically include the NUL-terminator in the copy length.
			*/
			auto bytesCount = std::strlen(name) + 1;
			std::memcpy(storage, name, bytesCount);
			return 0;
		}

		int pthread_getname_np(int, char* name, size_t len) {
			if (!name || 0 == len)
				return 1;
			/*
			* From GetThreadName we already know `name` is a null terminated array of
			* at least kMaxThreadNameBytes characters. We can safely copy it the
			* whole content of storage, which is also null terminated.
			*/
			std::memcpy(name, storage, std::min(kMaxThreadNameBytes, len));
			return 0;
		}
#else
#error Unsupported platform
#endif

	}

	size_t GetMaxThreadNameLength() {
		return kMaxThreadNameBytes - 1;
	}

	void SetThreadName(const std::string& name) {

		const size_t maxLength = GetMaxThreadNameLength();
		if (name.size() <= maxLength) {
			std::ignore = pthread_setname_np(pthread_self(), name.c_str());
			return;
		}

		// We truncate from the front of the name to preserve any unique suffixes,
		// which are often more useful for debugging than a common prefix.
		auto startOffset = name.size() > maxLength ? name.size() - maxLength : 0;
		auto truncatedName = name.substr(startOffset);
		std::ignore = pthread_setname_np(pthread_self(), truncatedName.c_str());
	}

	std::string GetThreadName() {
		char buffer[kMaxThreadNameBytes] = { '\0' };
		if (0 == pthread_getname_np(pthread_self(), buffer, sizeof(buffer))) {
			return std::string(buffer, buffer + std::strlen(buffer));
		}

		return {};
	}
}
}

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
		std::string name;
		name.resize(GetMaxThreadNameLength() + 1); // include space for NUL-terminator
		pthread_getname_np(pthread_self(), &name[0], name.size());
		return name.substr(0, name.find_first_of('\0'));
	}
}}

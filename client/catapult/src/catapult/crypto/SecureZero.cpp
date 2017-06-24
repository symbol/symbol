#include "SecureZero.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef _WIN32
#define ZEROMEM(PTR, SIZE) SecureZeroMemory(PTR, SIZE)
#elif defined(__APPLE__) || defined(__STDC_LIB_EXT1__)
#define ZEROMEM(PTR, SIZE) memset_s(PTR, SIZE, 0, SIZE);
#else
#define ZEROMEM(PTR, SIZE) \
	do { \
		volatile uint8_t* p = PTR; \
		size_t n = SIZE; \
		while (n--) *p++ = 0; \
	} while(0)
#endif

namespace catapult { namespace crypto {

	void SecureZero(Key& key) {
		SecureZero(&key[0], key.size());
	}

	void SecureZero(uint8_t* pData, size_t dataSize) {
		ZEROMEM(pData, dataSize);
	}
}}

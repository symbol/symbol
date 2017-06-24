#pragma once
#include "catapult/types.h"

namespace catapult { namespace crypto {

	/// Securely zeros out the memory backing the specified \a key.
	void SecureZero(Key& key);

	/// Securely zeros out the memory backing the specified \a pData with size \a dataSize.
	void SecureZero(uint8_t* pData, size_t dataSize);
}}

#pragma once
#include <stddef.h>

namespace catapult { namespace utils {

	/// Hasher object for an std::array with a variable offset.
	/// \note Offset defaults to 4 because because some arrays (e.g. Address) don't have a lot of entropy at the beginning.
	/// \note Hash is composed of only sizeof(size_t) bytes starting at offset.
	template<typename TArray, size_t Offset = 4>
	struct ArrayHasher {
		/// Hashes \a arrayData.
		size_t operator()(const TArray& arrayData) const {
			return reinterpret_cast<const size_t&>(arrayData[Offset]);
		}
	};

	/// Hasher object for a base value.
	template<typename TValue>
	struct BaseValueHasher {
		/// Hashes \a value.
		size_t operator()(TValue value) const {
			return static_cast<size_t>(value.unwrap());
		}
	};
}}

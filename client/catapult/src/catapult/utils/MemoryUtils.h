#pragma once
#include "catapult/exceptions.h"
#include <memory>

namespace catapult { namespace utils {

	/// Creates a unique pointer of the specified type with custom \a size.
	template<typename T>
	std::unique_ptr<T> MakeUniqueWithSize(size_t size) {
		if (size < sizeof(T))
			CATAPULT_THROW_INVALID_ARGUMENT("size is insufficient");

		return std::unique_ptr<T>(reinterpret_cast<T*>(::operator new(size)));
	}

	/// Creates a shared pointer of the specified type with custom \a size.
	template<typename T>
	std::shared_ptr<T> MakeSharedWithSize(size_t size) {
		if (size < sizeof(T))
			CATAPULT_THROW_INVALID_ARGUMENT("size is insufficient");

		return std::shared_ptr<T>(reinterpret_cast<T*>(::operator new(size)));
	}

	/// Converts a unique \a pointer to a shared pointer of the same type.
	template<typename T>
	std::shared_ptr<T> UniqueToShared(std::unique_ptr<T>&& pointer) {
		return std::move(pointer);
	}
}}

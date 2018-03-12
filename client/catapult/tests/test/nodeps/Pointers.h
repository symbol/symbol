#pragma once
#include <memory>
#include <vector>

namespace catapult { namespace test {

	/// Creates a vector of \a count unique pointers.
	template<typename T>
	std::vector<std::unique_ptr<T>> CreatePointers(size_t count) {
		std::vector<std::unique_ptr<T>> items;
		for (auto i = 0u; i < count; ++i)
			items.push_back(std::make_unique<T>());

		return items;
	}

	/// Creates a vector of raw pointers corresponding to the unique pointers in \a items.
	template<typename T>
	std::vector<T*> GetRawPointers(const std::vector<std::unique_ptr<T>>& items) {
		std::vector<T*> rawItems;
		for (const auto& pItem : items)
			rawItems.push_back(pItem.get());

		return rawItems;
	}
}}

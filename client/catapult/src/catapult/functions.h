#pragma once
#include <functional>

namespace catapult {

	/// An action function.
	using action = std::function<void ()>;

	/// A predicate function.
	template<typename... TArgs>
	using predicate = std::function<bool (TArgs...)>;

	/// A consumer function.
	template<typename... TArgs>
	using consumer = std::function<void (TArgs...)>;

	/// A (stateless) supplier function.
	template<typename T>
	using supplier = std::function<T ()>;
}

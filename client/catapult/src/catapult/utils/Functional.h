#pragma once

namespace catapult { namespace utils {

	/// Applies an accumulator function (\a fun) against an initial value(\a initialValue) and
	/// each element of \a container to reduce it to a single value.
	template<typename TContainer, typename TInitial, typename TFunction>
	auto Reduce(const TContainer& container, TInitial initialValue, TFunction fun) {
		for (const auto& element : container)
			initialValue = fun(initialValue, element);

		return initialValue;
	}
}}

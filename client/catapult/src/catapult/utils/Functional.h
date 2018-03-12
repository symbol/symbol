#pragma once
#include <type_traits>

namespace catapult { namespace utils {

	/// Applies an accumulator function (\a fun) against an initial value(\a initialValue) and
	/// each element of \a container to reduce it to a single value.
	template<typename TContainer, typename TInitial, typename TFunction>
	auto Reduce(const TContainer& container, TInitial initialValue, TFunction fun) {
		for (const auto& element : container)
			initialValue = fun(initialValue, element);

		return initialValue;
	}

	/// Applies \a accessor on each element of \a container and sums resulting values.
	/// \note this does not use Reduce in order to avoid creating another lambda.
	template<typename TContainer, typename TFunction>
	auto Sum(const TContainer& container, TFunction accessor) {
		std::result_of_t<TFunction(typename TContainer::value_type)> sum = 0;

		for (const auto& element : container)
			sum += accessor(element);

		return sum;
	}
}}

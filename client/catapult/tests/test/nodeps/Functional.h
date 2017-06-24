#pragma once
#include <vector>

namespace catapult { namespace test {

	/// Applies \a fun to all elements in \a container in the direction specified by \a forward.
	/// \see http://stackoverflow.com/questions/33379145/equivalent-of-python-map-function-using-lambda
	template<typename Container, typename Function>
	auto Apply(bool forward, const Container& container, Function fun) {
		std::vector<typename std::result_of<Function(const typename Container::value_type&)>::type> result;
		for (const auto& element : container)
			result.push_back(fun(element));

		if (!forward)
			std::reverse(result.begin(), result.end());

		return result;
	}

	/// Applies \a pred to all elements in \a container and returns all elements for which it returns \c true.
	template<typename Container, typename Predicate>
	auto Filter(const Container& container, Predicate pred) {
		Container result;
		for (const auto& element : container)
			if (pred(element))
				result.push_back(element);

		return result;
	}
}}

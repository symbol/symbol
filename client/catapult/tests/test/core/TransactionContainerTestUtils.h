#pragma once
#include <type_traits>

namespace catapult { namespace test {

	/// Returns the number of transactions in \a transactions.
	template<typename TContainer>
	size_t CountTransactions(TContainer transactions) {
		size_t count = 0;
		for (auto it = transactions.begin(); transactions.end() != it; ++it)
			++count;

		return count;
	}

	/// Traits for accessing a source via a const reference.
	template<typename TSource>
	struct ConstTraitsT {
		using SourceType = typename std::add_const<TSource>::type;

		/// Gets a const reference to \a source.
		static SourceType& GetAccessor(SourceType& source) {
			return source;
		}
	};

	/// Traits for accessing a source via a non-const reference.
	template<typename TSource>
	struct NonConstTraitsT {
		using SourceType = typename std::remove_const<TSource>::type;

		/// Gets a non-const reference to \a source.
		static SourceType& GetAccessor(SourceType& source) {
			return source;
		}
	};
}}

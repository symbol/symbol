#pragma once

namespace catapult { namespace utils {

	/// Removes all entries from \a map that fulfill the given \a predicate.
	template<typename TMap, typename TPredicate>
	void map_erase_if(TMap& map, TPredicate predicate) {
		for (auto iter = map.cbegin(); map.cend() != iter;) {
			if (predicate(*iter))
				iter = map.erase(iter);
			else
				++iter;
		}
	}
}}

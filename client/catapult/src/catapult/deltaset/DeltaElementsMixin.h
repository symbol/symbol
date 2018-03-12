#pragma once
#include "BaseSetDefaultTraits.h"
#include <unordered_set>

namespace catapult { namespace deltaset {

	/// Mixin that wraps BaseSetDelta and provides a facade on top of BaseSetDelta::deltas().
	/// \note This only works for map-based delta sets.
	template<typename TSetDelta>
	class DeltaElementsMixin {
	private:
		using DerefHelper = detail::DerefHelper<typename TSetDelta::SetType::value_type::second_type>;
		using PointerContainer = std::unordered_set<typename DerefHelper::const_pointer_type>;

	public:
		/// Creates a mixin around \a setDelta.
		explicit DeltaElementsMixin(const TSetDelta& setDelta) : m_setDelta(setDelta)
		{}

	public:
		/// Gets pointers to all added elements.
		auto addedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Added);
		}

		/// Gets pointers to all modified elements.
		auto modifiedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Copied);
		}

		/// Gets pointers to all removed elements.
		auto removedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Removed);
		}

	private:
		template<typename TSource>
		static PointerContainer CollectAllPointers(const TSource& source) {
			PointerContainer dest;
			for (const auto& pair : source)
				dest.insert(&DerefHelper::Deref(pair.second));

			return dest;
		}

	private:
		const TSetDelta& m_setDelta;
	};
}}

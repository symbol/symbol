#pragma once
#include "src/types.h"
#include "catapult/utils/Hashers.h"
#include "catapult/exceptions.h"
#include <unordered_set>

namespace catapult { namespace state {

	/// A collection of mosaic ids that have a key in common.
	template<typename TKey>
	class GroupedMosaicIds {
	public:
		using MosaicIds = std::unordered_set<MosaicId, utils::BaseValueHasher<MosaicId>>;

	public:
		/// Creates a container around a given \a key.
		explicit GroupedMosaicIds(TKey key) : m_key(key)
		{}

	public:
		/// Gets the key.
		TKey key() const {
			return m_key;
		}

		/// Gets the mosaic ids.
		const MosaicIds& mosaicIds() const {
			return m_mosaicIds;
		}

	public:
		/// Returns \c true if the set of mosaic ids is empty, \c false otherwise.
		bool empty() const {
			return m_mosaicIds.empty();
		}

		/// Gets the number of mosaic ids in the set.
		size_t size() const {
			return m_mosaicIds.size();
		}

	public:
		/// Adds \a mosaicId to the set.
		void add(MosaicId mosaicId) {
			m_mosaicIds.insert(mosaicId);
		}

		/// Removes \a mosaicId from the set.
		void remove(MosaicId mosaicId) {
			m_mosaicIds.erase(mosaicId);
		}

	private:
		TKey m_key;
		MosaicIds m_mosaicIds;
	};

	/// Mosaics grouped by namespace id.
	using NamespaceMosaics = GroupedMosaicIds<NamespaceId>;

	/// Mosaics grouped by expiry height.
	using HeightMosaics = GroupedMosaicIds<Height>;
}}

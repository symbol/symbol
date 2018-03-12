#pragma once
#include "catapult/model/Block.h"
#include "catapult/model/WeakEntityInfo.h"

namespace catapult { namespace test {

	/// Wraps a vector of entity infos and their backing memory.
	class EntityInfoContainerWrapper {
	public:
		/// Creates a wrapper around \a count entity infos.
		explicit EntityInfoContainerWrapper(size_t count);

	public:
		/// Returns a vector containing all entity infos.
		model::WeakEntityInfos toVector() const;

	private:
		std::shared_ptr<model::Block> m_pBlock;
		model::BasicContiguousEntityContainer<model::Transaction> m_container;
		std::vector<Hash256> m_hashes;
	};

	/// Creates a wrapper around \a count entity infos such that the entity infos have incrementing timestamps.
	EntityInfoContainerWrapper CreateEntityInfos(size_t count);
}}

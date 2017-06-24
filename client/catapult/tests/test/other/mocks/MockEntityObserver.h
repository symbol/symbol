#pragma once
#include "catapult/model/Block.h"
#include "catapult/observers/ObserverTypes.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Mock entity observer that captures information about observed entities and contexts.
	class MockEntityObserver : public observers::EntityObserver {
	public:
		/// Creates a mock observer with a default name,
		MockEntityObserver() : MockEntityObserver("MockObserverT")
		{}

		/// Creates a mock observer with \a name.
		explicit MockEntityObserver(const std::string& name) : m_name(name)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		void notify(const model::WeakEntityInfo& entityInfo, const observers::ObserverContext& context) const override {
			m_entityHashes.push_back(entityInfo.hash());
			m_contexts.push_back(context);

			const auto& entity = entityInfo.entity();
			m_versions.push_back(entity.Version);
			if (model::BasicEntityType::Block == model::ToBasicEntityType(entity.Type))
				m_blockHeights.push_back(static_cast<const model::Block&>(entity).Height);
		}

	public:
		/// Returns collected block heights.
		const auto& blockHeights() const {
			return m_blockHeights;
		}

		/// Returns collected versions.
		const auto& versions() const {
			return m_versions;
		}

		/// Returns collected entity hashes.
		const auto& entityHashes() const {
			return m_entityHashes;
		}

		/// Returns collected contexts.
		const auto& contexts() const {
			return m_contexts;
		}

	private:
		std::string m_name;
		mutable std::vector<Height> m_blockHeights;
		mutable std::vector<uint16_t> m_versions;
		mutable std::vector<Hash256> m_entityHashes;
		mutable std::vector<observers::ObserverContext> m_contexts;
	};
}}

/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/cache_db/CacheDatabase.h"
#include "catapult/cache_db/PatriciaTreeRdbDataSource.h"
#include <memory>

namespace catapult { namespace cache {

	/// Wrapper around a patricia tree used by caches.
	template<typename TTree>
	class CachePatriciaTree {
	public:
		/// Creates a tree around \a database and \a columnId if \a enable is \c true.
		CachePatriciaTree(bool enable, CacheDatabase& database, size_t columnId)
				: m_pImpl(enable ? std::make_unique<Impl>(database, columnId) : nullptr)
		{}

	public:
		/// Gets a pointer to the underlying tree if enabled.
		const auto* get() const {
			return m_pImpl ? &m_pImpl->tree() : nullptr;
		}

	public:
		/// Gets a delta based on the same data source as this tree.
		auto rebase() {
			return m_pImpl ? m_pImpl->tree().rebase() : nullptr;
		}

		/// Gets a delta based on the same data source as this tree
		/// but without the ability to commit any changes to the original tree.
		auto rebaseDetached() const {
			return m_pImpl ? m_pImpl->tree().rebaseDetached() : nullptr;
		}

		/// Commits all changes in the rebased tree.
		void commit() {
			if (m_pImpl)
				m_pImpl->commit();
		}

	private:
		class Impl {
		public:
			Impl(CacheDatabase& database, size_t columnId)
					: m_container(database, columnId)
					, m_dataSource(m_container)
					, m_pTree(std::make_unique<TTree>(m_dataSource)) {
				Hash256 rootHash;
				if (!m_container.prop("root", rootHash))
					return;

				if (Hash256() == rootHash)
					return;

				m_pTree = std::make_unique<TTree>(m_dataSource, rootHash);
			}

		public:
			const auto& tree() const {
				return *m_pTree;
			}

			auto& tree() {
				return *m_pTree;
			}

		public:
			void commit() {
				m_pTree->commit();

				// skip setProp if hash did not change
				Hash256 rootHash;
				if (m_container.prop("root", rootHash) && rootHash == m_pTree->root())
					return;

				m_container.setProp("root", m_pTree->root());
			}

		private:
			PatriciaTreeContainer m_container;
			PatriciaTreeRdbDataSource m_dataSource;
			std::unique_ptr<TTree> m_pTree;
		};

		std::unique_ptr<Impl> m_pImpl;
	};
}}

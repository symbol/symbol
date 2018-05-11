/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "catapult/functions.h"

namespace catapult {
	namespace cache { class CatapultCacheDelta; }
	namespace extensions { struct LocalNodeStateRef; }
	namespace model {
		struct BlockChainConfiguration;
		struct BlockElement;
		class NotificationPublisher;
		class TransactionRegistry;
	}
	namespace observers { class EntityObserver; }
	namespace state { struct CatapultState; }
}

namespace catapult { namespace extensions {

	/// Loads and executes a nemesis block.
	class NemesisBlockLoader {
	public:
		/// Creates a loader around \a transactionRegistry, \a publisher and \a observer.
		NemesisBlockLoader(
				const model::TransactionRegistry& transactionRegistry,
				const model::NotificationPublisher& publisher,
				const observers::EntityObserver& observer);

	public:
		/// Loads the nemesis block from storage, updates state in \a stateRef and commits all changes.
		void executeAndCommit(const LocalNodeStateRef& stateRef) const;

		/// Executes the nemesis block (\a nemesisBlockElement), applies all changes to \a cacheDelta and checks consistency
		/// against \a config.
		/// \note Execution uses a default catapult state.
		void execute(
				const model::BlockChainConfiguration& config,
				const model::BlockElement& nemesisBlockElement,
				cache::CatapultCacheDelta& cacheDelta) const;

	private:
		enum class Verbosity { Off, On };

		void execute(
				const model::BlockChainConfiguration& config,
				const model::BlockElement& nemesisBlockElement,
				cache::CatapultCacheDelta& cacheDelta,
				state::CatapultState& catapultState,
				Verbosity verbosity) const;

	private:
		const model::TransactionRegistry& m_transactionRegistry;
		const model::NotificationPublisher& m_publisher;
		const observers::EntityObserver& m_observer;
	};
}}

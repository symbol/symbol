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

	/// State hash verification.
	enum class StateHashVerification {
		/// State hash verification disabled.
		Disabled,

		/// State hash verification enabled.
		Enabled
	};

	/// Loads and executes a nemesis block.
	class NemesisBlockLoader {
	public:
		/// Creates a loader around \a cacheDelta, \a transactionRegistry, \a publisher and \a observer.
		NemesisBlockLoader(
				cache::CatapultCacheDelta& cacheDelta,
				const model::TransactionRegistry& transactionRegistry,
				const model::NotificationPublisher& publisher,
				const observers::EntityObserver& observer);

	public:
		/// Loads the nemesis block from storage, updates state in \a stateRef and verifies state hash (\a stateHashVerification).
		void execute(const LocalNodeStateRef& stateRef, StateHashVerification stateHashVerification) const;

		/// Loads the nemesis block from storage, updates state in \a stateRef optionally verifying state hash (\a stateHashVerification)
		/// and commits all changes to cache.
		void executeAndCommit(
				const LocalNodeStateRef& stateRef,
				StateHashVerification stateHashVerification = StateHashVerification::Enabled) const;

		/// Executes the nemesis block (\a nemesisBlockElement), applies all changes to cache delta and checks consistency
		/// against \a config.
		/// \note Execution uses a default catapult state.
		void execute(const model::BlockChainConfiguration& config, const model::BlockElement& nemesisBlockElement) const;

	private:
		enum class Verbosity { Off, On };

		void execute(
				const model::BlockChainConfiguration& config,
				const model::BlockElement& nemesisBlockElement,
				state::CatapultState& catapultState,
				StateHashVerification stateHashVerification,
				Verbosity verbosity) const;

	private:
		cache::CatapultCacheDelta& m_cacheDelta;
		const model::TransactionRegistry& m_transactionRegistry;
		const model::NotificationPublisher& m_publisher;
		const observers::EntityObserver& m_observer;
	};
}}

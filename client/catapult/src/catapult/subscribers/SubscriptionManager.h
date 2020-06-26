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
#include "FinalizationSubscriber.h"
#include "NodeSubscriber.h"
#include "StateChangeSubscriber.h"
#include "TransactionStatusSubscriber.h"
#include "catapult/cache_tx/MemoryPtCache.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/cache_tx/PtChangeSubscriber.h"
#include "catapult/cache_tx/UtChangeSubscriber.h"
#include "catapult/io/BlockChangeSubscriber.h"
#include "catapult/io/FileBlockStorage.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace config { class CatapultConfiguration; } }

namespace catapult { namespace subscribers {

	/// Manager for subscribing to notifications.
	class SubscriptionManager {
	public:
		/// Creates a new subscription manager around \a config.
		explicit SubscriptionManager(const config::CatapultConfiguration& config);

	public:
		/// Gets the underlying file storage.
		io::BlockStorage& fileStorage();

	public:
		/// Registers a block change subscriber (\a pSubscriber).
		void addBlockChangeSubscriber(std::unique_ptr<io::BlockChangeSubscriber>&& pSubscriber);

		/// Registers a partial transactions change subscriber (\a pSubscriber).
		void addPtChangeSubscriber(std::unique_ptr<cache::PtChangeSubscriber>&& pSubscriber);

		/// Registers an unconfirmed transactions change subscriber (\a pSubscriber).
		void addUtChangeSubscriber(std::unique_ptr<cache::UtChangeSubscriber>&& pSubscriber);

		/// Adds a finalization subscriber (\a pSubscriber).
		void addFinalizationSubscriber(std::unique_ptr<FinalizationSubscriber>&& pSubscriber);

		/// Adds a node subscriber (\a pSubscriber).
		void addNodeSubscriber(std::unique_ptr<NodeSubscriber>&& pSubscriber);

		/// Adds a state change subscriber (\a pSubscriber).
		void addStateChangeSubscriber(std::unique_ptr<StateChangeSubscriber>&& pSubscriber);

		/// Adds a transaction status subscriber (\a pSubscriber).
		void addTransactionStatusSubscriber(std::unique_ptr<TransactionStatusSubscriber>&& pSubscriber);

	public:
		/// Creates the block change subscriber.
		std::unique_ptr<io::BlockChangeSubscriber> createBlockChangeSubscriber();

		/// Creates the pt change subscriber.
		std::unique_ptr<cache::PtChangeSubscriber> createPtChangeSubscriber();

		/// Creates the ut change subscriber.
		std::unique_ptr<cache::UtChangeSubscriber> createUtChangeSubscriber();

		/// Creates the finalization subscriber.
		std::unique_ptr<FinalizationSubscriber> createFinalizationSubscriber();

		/// Creates the node subscriber.
		std::unique_ptr<NodeSubscriber> createNodeSubscriber();

		/// Creates the state change subscriber.
		std::unique_ptr<StateChangeSubscriber> createStateChangeSubscriber();

		/// Creates the transaction status subscriber.
		std::unique_ptr<TransactionStatusSubscriber> createTransactionStatusSubscriber();

	public:
		/// Creates the block storage and sets \a pSubscriber to the created block change subscriber.
		/// \note createBlockChangeSubscriber cannot be called if this function is called.
		std::unique_ptr<io::BlockStorage> createBlockStorage(io::BlockChangeSubscriber*& pSubscriber);

		/// Creates the partial transactions cache with the specified cache \a options.
		/// \note createPtChangeSubscriber cannot be called if this function is called.
		std::unique_ptr<cache::MemoryPtCacheProxy> createPtCache(const cache::MemoryCacheOptions& options);

		/// Creates the unconfirmed transactions cache with the specified cache \a options.
		/// \note createUtChangeSubscriber cannot be called if this function is called.
		std::unique_ptr<cache::MemoryUtCacheProxy> createUtCache(const cache::MemoryCacheOptions& options);

	private:
		enum class SubscriberType : uint32_t {
			Block_Change,
			Pt_Change,
			Ut_Change,
			Finalization,
			Node,
			State_Change,
			Transaction_Status,
			Count
		};

	private:
		void requireUnused(SubscriberType subscriberType) const;

		void markUsed(SubscriberType subscriberType);

	private:
		const config::CatapultConfiguration& m_config;
		std::unique_ptr<io::FileBlockStorage> m_pStorage;
		std::array<bool, utils::to_underlying_type(SubscriberType::Count)> m_subscriberUsedFlags;

		std::vector<std::unique_ptr<io::BlockChangeSubscriber>> m_blockChangeSubscribers;
		std::vector<std::unique_ptr<cache::PtChangeSubscriber>> m_ptChangeSubscribers;
		std::vector<std::unique_ptr<cache::UtChangeSubscriber>> m_utChangeSubscribers;
		std::vector<std::unique_ptr<FinalizationSubscriber>> m_finalizationSubscribers;
		std::vector<std::unique_ptr<NodeSubscriber>> m_nodeSubscribers;
		std::vector<std::unique_ptr<StateChangeSubscriber>> m_stateChangeSubscribers;
		std::vector<std::unique_ptr<TransactionStatusSubscriber>> m_transactionStatusSubscribers;
	};
}}

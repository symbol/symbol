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

#include "SubscriptionManager.h"
#include "AggregateBlockChangeSubscriber.h"
#include "AggregateNodeSubscriber.h"
#include "AggregatePtChangeSubscriber.h"
#include "AggregateStateChangeSubscriber.h"
#include "AggregateTransactionStatusSubscriber.h"
#include "AggregateUtChangeSubscriber.h"
#include "catapult/cache_tx/AggregatePtCache.h"
#include "catapult/cache_tx/AggregateUtCache.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/io/AggregateBlockStorage.h"

namespace catapult { namespace subscribers {

	SubscriptionManager::SubscriptionManager(const config::CatapultConfiguration& config)
			: m_config(config)
			, m_pStorage(std::make_unique<io::FileBlockStorage>(m_config.User.DataDirectory)) {
		m_subscriberUsedFlags.fill(false);
	}

	io::BlockStorage& SubscriptionManager::fileStorage() {
		return *m_pStorage;
	}

	// region add - subscriber

	void SubscriptionManager::addBlockChangeSubscriber(std::unique_ptr<io::BlockChangeSubscriber>&& pSubscriber) {
		requireUnused(SubscriberType::BlockChange);
		m_blockChangeSubscribers.push_back(std::move(pSubscriber));
	}

	void SubscriptionManager::addUtChangeSubscriber(std::unique_ptr<cache::UtChangeSubscriber>&& pSubscriber) {
		requireUnused(SubscriberType::UtChange);
		m_utChangeSubscribers.push_back(std::move(pSubscriber));
	}

	void SubscriptionManager::addPtChangeSubscriber(std::unique_ptr<cache::PtChangeSubscriber>&& pSubscriber) {
		requireUnused(SubscriberType::PtChange);
		m_ptChangeSubscribers.push_back(std::move(pSubscriber));
	}

	void SubscriptionManager::addTransactionStatusSubscriber(std::unique_ptr<TransactionStatusSubscriber>&& pSubscriber) {
		requireUnused(SubscriberType::TransactionStatus);
		m_transactionStatusSubscribers.push_back(std::move(pSubscriber));
	}

	void SubscriptionManager::addStateChangeSubscriber(std::unique_ptr<StateChangeSubscriber>&& pSubscriber) {
		requireUnused(SubscriberType::StateChange);
		m_stateChangeSubscribers.push_back(std::move(pSubscriber));
	}

	void SubscriptionManager::addNodeSubscriber(std::unique_ptr<NodeSubscriber>&& pSubscriber) {
		requireUnused(SubscriberType::Node);
		m_nodeSubscribers.push_back(std::move(pSubscriber));
	}

	// endregion

	// region create - subscriber

	std::unique_ptr<io::BlockChangeSubscriber> SubscriptionManager::createBlockChangeSubscriber() {
		markUsed(SubscriberType::BlockChange);
		return std::make_unique<AggregateBlockChangeSubscriber<>>(std::move(m_blockChangeSubscribers));
	}

	std::unique_ptr<cache::UtChangeSubscriber> SubscriptionManager::createUtChangeSubscriber() {
		markUsed(SubscriberType::UtChange);
		return std::make_unique<AggregateUtChangeSubscriber<>>(std::move(m_utChangeSubscribers));
	}

	std::unique_ptr<cache::PtChangeSubscriber> SubscriptionManager::createPtChangeSubscriber() {
		markUsed(SubscriberType::PtChange);
		return std::make_unique<AggregatePtChangeSubscriber<>>(std::move(m_ptChangeSubscribers));
	}

	std::unique_ptr<TransactionStatusSubscriber> SubscriptionManager::createTransactionStatusSubscriber() {
		class LoggingTransactionStatusSubscriber : public TransactionStatusSubscriber {
		public:
			void notifyStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) override {
				auto result = validators::ValidationResult(status);
				CATAPULT_LOG_LEVEL(validators::MapToLogLevel(result))
						<< "rejected tx " << hash << " due to result " << result
						<< " (deadline " << transaction.Deadline << ")";
			}

			void flush() override
			{}
		};

		markUsed(SubscriberType::TransactionStatus);
		m_transactionStatusSubscribers.push_back(std::make_unique<LoggingTransactionStatusSubscriber>());
		return std::make_unique<AggregateTransactionStatusSubscriber<>>(std::move(m_transactionStatusSubscribers));
	}

	std::unique_ptr<StateChangeSubscriber> SubscriptionManager::createStateChangeSubscriber() {
		markUsed(SubscriberType::StateChange);
		return std::make_unique<AggregateStateChangeSubscriber<>>(std::move(m_stateChangeSubscribers));
	}

	std::unique_ptr<NodeSubscriber> SubscriptionManager::createNodeSubscriber() {
		markUsed(SubscriberType::Node);
		return std::make_unique<AggregateNodeSubscriber<>>(std::move(m_nodeSubscribers));
	}

	// endregion

	// region create - container

	std::unique_ptr<io::BlockStorage> SubscriptionManager::createBlockStorage(io::BlockChangeSubscriber*& pSubscriber) {
		if (!m_blockChangeSubscribers.empty()) {
			auto pBlockChangeSubscriber = createBlockChangeSubscriber();
			pSubscriber = pBlockChangeSubscriber.get();
			return io::CreateAggregateBlockStorage(std::move(m_pStorage), std::move(pBlockChangeSubscriber));
		}

		markUsed(SubscriberType::BlockChange);
		pSubscriber = nullptr;
		return std::move(m_pStorage);
	}

	std::unique_ptr<cache::MemoryUtCacheProxy> SubscriptionManager::createUtCache(const cache::MemoryCacheOptions& options) {
		if (!m_utChangeSubscribers.empty())
			return std::make_unique<cache::MemoryUtCacheProxy>(options, cache::CreateAggregateUtCache, createUtChangeSubscriber());

		markUsed(SubscriberType::UtChange);
		return std::make_unique<cache::MemoryUtCacheProxy>(options);
	}

	std::unique_ptr<cache::MemoryPtCacheProxy> SubscriptionManager::createPtCache(const cache::MemoryCacheOptions& options) {
		if (!m_ptChangeSubscribers.empty())
			return std::make_unique<cache::MemoryPtCacheProxy>(options, cache::CreateAggregatePtCache, createPtChangeSubscriber());

		markUsed(SubscriberType::PtChange);
		return std::make_unique<cache::MemoryPtCacheProxy>(options);
	}

	// endregion

	void SubscriptionManager::requireUnused(SubscriberType subscriberType) const {
		auto rawSubscriberType = utils::to_underlying_type(subscriberType);
		if (!m_subscriberUsedFlags[rawSubscriberType])
			return;

		std::ostringstream out;
		out << "subscription aggregate has already been created for subscription type " << rawSubscriberType;
		CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
	}

	void SubscriptionManager::markUsed(SubscriberType subscriberType) {
		requireUnused(subscriberType);
		m_subscriberUsedFlags[utils::to_underlying_type(subscriberType)] = true;
	}
}}

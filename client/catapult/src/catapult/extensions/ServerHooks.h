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
#include "BasicServerHooks.h"
#include "TransactionEvent.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/chain/ChainFunctions.h"
#include "catapult/chain/ChainSynchronizer.h"
#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/InputUtils.h"
#include "catapult/disruptor/DisruptorElement.h"
#include "catapult/handlers/HandlerTypes.h"
#include "catapult/ionet/PacketPayload.h"

namespace catapult { namespace extensions {

	// region function aliases

	/// New block sink prototype.
	using NewBlockSink = consumers::NewBlockSink;

	/// New transactions sink prototype that does not take ownership of new infos.
	using SharedNewTransactionsSink = consumer<const consumers::TransactionInfos&>;

	/// Packet payload sink.
	using PacketPayloadSink = consumer<const ionet::PacketPayload&>;

	/// Banned node identity sink.
	using BannedNodeIdentitySink = consumer<const model::NodeIdentity&>;

	/// Handler that is called when the confirmed state of transactions changes.
	using TransactionsChangeHandler = consumers::BlockChainSyncHandlers::TransactionsChangeFunc;

	/// Function signature for delivering a block range to a consumer.
	using BlockRangeConsumerFunc = handlers::BlockRangeHandler;

	/// Function signature for delivering a transaction range to a consumer.
	using TransactionRangeConsumerFunc = handlers::TransactionRangeHandler;

	/// Factory for creating a range consumer function bound to an input source.
	template<typename TConsumer>
	using RangeConsumerFactoryFunc = std::function<TConsumer (disruptor::InputSource)>;

	/// Factory for creating a BlockRangeConsumerFunc bound to an input source.
	using BlockRangeConsumerFactoryFunc = RangeConsumerFactoryFunc<BlockRangeConsumerFunc>;

	/// Factory for creating a CompletionAwareBlockRangeConsumerFunc bound to an input source.
	using CompletionAwareBlockRangeConsumerFactoryFunc = RangeConsumerFactoryFunc<chain::CompletionAwareBlockRangeConsumerFunc>;

	/// Factory for creating a TransactionRangeConsumerFunc bound to an input source.
	using TransactionRangeConsumerFactoryFunc = RangeConsumerFactoryFunc<TransactionRangeConsumerFunc>;

	/// Retriever that returns the network chain heights for a number of peers.
	using RemoteChainHeightsRetriever = std::function<thread::future<std::vector<Height>> (size_t)>;

	/// Supplier for retrieving the local finalized height hash pair.
	using LocalFinalizedHeightHashPairSupplier = supplier<model::HeightHashPair>;

	/// Predicate for determining if a chain is synced.
	using ChainSyncedPredicate = predicate<>;

	/// Predicate for determining if a hash is known.
	using KnownHashPredicate = chain::KnownHashPredicate;

	// endregion

	/// Hooks that can be used to configure server behavior.
	class ServerHooks {
	public:
		/// Adds a new block \a sink.
		void addNewBlockSink(const NewBlockSink& sink) {
			m_newBlockSinks.push_back(sink);
		}

		/// Adds a new transactions \a sink.
		void addNewTransactionsSink(const SharedNewTransactionsSink& sink) {
			m_newTransactionsSinks.push_back(sink);
		}

		/// Adds a packet payload \a sink.
		void addPacketPayloadSink(const PacketPayloadSink& sink) {
			m_packetPayloadSinks.push_back(sink);
		}

		/// Adds a banned node identity \a sink.
		void addBannedNodeIdentitySink(const BannedNodeIdentitySink& sink) {
			m_bannedNodeIdentitySinks.push_back(sink);
		}

		/// Adds a transactions change \a handler.
		void addTransactionsChangeHandler(const TransactionsChangeHandler& handler) {
			m_transactionsChangeHandlers.push_back(handler);
		}

		/// Adds a transaction event \a handler.
		void addTransactionEventHandler(const TransactionEventHandler& handler) {
			m_transactionEventHandlers.push_back(handler);
		}

		/// Sets the \a factory for creating a BlockRangeConsumerFunc bound to an input source.
		void setBlockRangeConsumerFactory(const BlockRangeConsumerFactoryFunc& factory) {
			SetOnce(m_blockRangeConsumerFactory, factory);
		}

		/// Sets the \a factory for creating a CompletionAwareBlockRangeConsumerFunc bound to an input source.
		void setCompletionAwareBlockRangeConsumerFactory(const CompletionAwareBlockRangeConsumerFactoryFunc& factory) {
			SetOnce(m_completionAwareBlockRangeConsumerFactory, factory);
		}

		/// Sets the \a factory for creating a TransactionRangeConsumerFunc bound to an input source.
		void setTransactionRangeConsumerFactory(const TransactionRangeConsumerFactoryFunc& factory) {
			SetOnce(m_transactionRangeConsumerFactory, factory);
		}

		/// Sets the remote chain heights \a retriever.
		void setRemoteChainHeightsRetriever(const RemoteChainHeightsRetriever& retriever) {
			SetOnce(m_remoteChainHeightsRetriever, retriever);
		}

		/// Sets the local finalized height hash pair \a supplier.
		void setLocalFinalizedHeightHashPairSupplier(const LocalFinalizedHeightHashPairSupplier& supplier) {
			SetOnce(m_localFinalizedHeightHashPairSupplier, supplier);
		}

		/// Sets the chain synced \a predicate.
		void setChainSyncedPredicate(const ChainSyncedPredicate& predicate) {
			SetOnce(m_chainSyncedPredicate, predicate);
		}

		/// Adds a known hash \a predicate.
		void addKnownHashPredicate(const KnownHashPredicate& predicate) {
			m_knownHashPredicates.push_back(predicate);
		}

	public:
		/// Gets the new block sink.
		auto newBlockSink() const {
			return AggregateConsumers(m_newBlockSinks);
		}

		/// Gets the new transactions sink.
		auto newTransactionsSink() const {
			return AggregateConsumers(m_newTransactionsSinks);
		}

		/// Gets the packet payload sink.
		auto packetPayloadSink() const {
			return AggregateConsumers(m_packetPayloadSinks);
		}

		/// Gets the banned node identity sink.
		auto bannedNodeIdentitySink() const {
			return AggregateConsumers(m_bannedNodeIdentitySinks);
		}

		/// Gets the transactions change handler.
		auto transactionsChangeHandler() const {
			return AggregateConsumers(m_transactionsChangeHandlers);
		}

		/// Gets the transaction event handler.
		auto transactionEventHandler() const {
			return AggregateConsumers(m_transactionEventHandlers);
		}

		/// Gets the factory for creating a BlockRangeConsumerFunc bound to an input source.
		const auto& blockRangeConsumerFactory() const {
			return Require(m_blockRangeConsumerFactory);
		}

		/// Gets the factory for creating a CompletionAwareBlockRangeConsumerFunc bound to an input source.
		const auto& completionAwareBlockRangeConsumerFactory() const {
			return Require(m_completionAwareBlockRangeConsumerFactory);
		}

		/// Gets the factory for creating a TransactionRangeConsumerFunc bound to an input source.
		const auto& transactionRangeConsumerFactory() const {
			return Require(m_transactionRangeConsumerFactory);
		}

		/// Gets the remote chain heights retriever.
		const auto& remoteChainHeightsRetriever() const {
			return Require(m_remoteChainHeightsRetriever);
		}

		/// Gets the local finalized height hash pair supplier.
		auto localFinalizedHeightHashPairSupplier() const {
			return Require(m_localFinalizedHeightHashPairSupplier);
		}

		/// Gets the chain synced predicate.
		auto chainSyncedPredicate() const {
			return m_chainSyncedPredicate ? m_chainSyncedPredicate : []() { return true; };
		}

		/// Gets the known hash predicate augmented with a check in \a utCache.
		KnownHashPredicate knownHashPredicate(const cache::ReadWriteUtCache& utCache) const {
			return [&utCache, knownHashPredicates = m_knownHashPredicates](auto timestamp, const auto& hash) {
				if (utCache.view().contains(hash))
					return true;

				for (const auto& knownHashPredicate : knownHashPredicates) {
					if (knownHashPredicate(timestamp, hash))
						return true;
				}

				return false;
			};
		}

	private:
		std::vector<NewBlockSink> m_newBlockSinks;
		std::vector<SharedNewTransactionsSink> m_newTransactionsSinks;
		std::vector<PacketPayloadSink> m_packetPayloadSinks;
		std::vector<BannedNodeIdentitySink> m_bannedNodeIdentitySinks;
		std::vector<TransactionsChangeHandler> m_transactionsChangeHandlers;
		std::vector<TransactionEventHandler> m_transactionEventHandlers;

		BlockRangeConsumerFactoryFunc m_blockRangeConsumerFactory;
		CompletionAwareBlockRangeConsumerFactoryFunc m_completionAwareBlockRangeConsumerFactory;
		TransactionRangeConsumerFactoryFunc m_transactionRangeConsumerFactory;

		RemoteChainHeightsRetriever m_remoteChainHeightsRetriever;
		LocalFinalizedHeightHashPairSupplier m_localFinalizedHeightHashPairSupplier;
		ChainSyncedPredicate m_chainSyncedPredicate;
		std::vector<KnownHashPredicate> m_knownHashPredicates;
	};
}}

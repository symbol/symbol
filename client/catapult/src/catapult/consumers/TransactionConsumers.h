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
#include "HashCheckOptions.h"
#include "InputUtils.h"
#include "catapult/chain/ChainFunctions.h"
#include "catapult/crypto/Signer.h"
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/validators/ParallelValidationPolicy.h"

namespace catapult { namespace model { class NotificationPublisher; } }

namespace catapult { namespace consumers {

	/// Creates a consumer that calculates hashes of all entities using \a transactionRegistry for the network with the specified
	/// generation hash seed (\a generationHashSeed).
	disruptor::TransactionConsumer CreateTransactionHashCalculatorConsumer(
			const GenerationHashSeed& generationHashSeed,
			const model::TransactionRegistry& transactionRegistry);

	/// Creates a consumer that checks entities for previous processing based on their hash.
	/// \a timeSupplier is used for generating timestamps and \a options specifies additional cache options.
	/// \a knownHashPredicate returns \c true for known hashes.
	disruptor::TransactionConsumer CreateTransactionHashCheckConsumer(
			const chain::TimeSupplier& timeSupplier,
			const HashCheckOptions& options,
			const chain::KnownHashPredicate& knownHashPredicate);

	/// Creates a consumer that runs stateless validation using \a pValidationPolicy and calls \a failedTransactionSink for each failure.
	disruptor::TransactionConsumer CreateTransactionStatelessValidationConsumer(
			const std::shared_ptr<const validators::ParallelValidationPolicy>& pValidationPolicy,
			const chain::FailedTransactionSink& failedTransactionSink);

	/// Creates a consumer that runs batch signature validation using \a pPublisher and \a pool for the network with the specified
	/// generation hash seed (\a generationHashSeed) and calls \a failedTransactionSink for each failure.
	/// \a randomFiller is used to generate random bytes.
	disruptor::TransactionConsumer CreateTransactionBatchSignatureConsumer(
			const GenerationHashSeed& generationHashSeed,
			const crypto::RandomFiller& randomFiller,
			const std::shared_ptr<const model::NotificationPublisher>& pPublisher,
			thread::IoThreadPool& pool,
			const chain::FailedTransactionSink& failedTransactionSink);

	/// Prototype for a function that is called with new transactions.
	using NewTransactionsSink = consumer<TransactionInfos&&>;

	/// Creates a consumer that calls \a newTransactionsSink with all new transactions.
	/// \note This consumer must be last because it destroys the input.
	disruptor::DisruptorConsumer CreateNewTransactionsConsumer(const NewTransactionsSink& newTransactionsSink);
}}

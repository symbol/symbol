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
#include "catapult/io/BlockChangeSubscriber.h"
#include "catapult/subscribers/FinalizationSubscriber.h"
#include "catapult/subscribers/NodeSubscriber.h"
#include "catapult/subscribers/StateChangeSubscriber.h"
#include "catapult/subscribers/TransactionStatusSubscriber.h"
#include "tests/test/cache/UnsupportedTransactionsChangeSubscribers.h"

namespace catapult { namespace test {

	/// Unsupported block change subscriber.
	class UnsupportedBlockChangeSubscriber : public io::BlockChangeSubscriber {
	public:
		void notifyBlock(const model::BlockElement&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyBlock - not supported in mock");
		}

		void notifyDropBlocksAfter(Height) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyDropBlocksAfter - not supported in mock");
		}
	};

	/// Unsupported finalization subscriber.
	class UnsupportedFinalizationSubscriber : public subscribers::FinalizationSubscriber {
	public:
		void notifyFinalizedBlock(Height, const Hash256&, FinalizationPoint) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyFinalizedBlock - not supported in mock");
		}
	};

	/// Unsupported node subscriber.
	class UnsupportedNodeSubscriber : public subscribers::NodeSubscriber {
	public:
		void notifyNode(const ionet::Node&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyNode - not supported in mock");
		}

		bool notifyIncomingNode(const model::NodeIdentity&, ionet::ServiceIdentifier) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyIncomingNode - not supported in mock");
		}

		void notifyBan(const model::NodeIdentity&, uint32_t) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyBan - not supported in mock");
		}
	};

	/// Unsupported state change subscriber.
	class UnsupportedStateChangeSubscriber : public subscribers::StateChangeSubscriber {
	public:
		void notifyScoreChange(const model::ChainScore&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyScoreChange - not supported in mock");
		}

		void notifyStateChange(const subscribers::StateChangeInfo&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyStateChange - not supported in mock");
		}
	};

	/// Unsupported transaction status subscriber.
	template<UnsupportedFlushBehavior FlushBehavior>
	class UnsupportedTransactionStatusSubscriber : public subscribers::TransactionStatusSubscriber {
	public:
		void notifyStatus(const model::Transaction&, const Hash256&, uint32_t) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyStatus - not supported in mock");
		}

		void flush() override {
			FlushInvoker<FlushBehavior>::Flush();
		}
	};
}}

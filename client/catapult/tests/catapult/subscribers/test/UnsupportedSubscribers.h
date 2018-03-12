#pragma once
#include "catapult/io/BlockChangeSubscriber.h"
#include "catapult/subscribers/NodeSubscriber.h"
#include "catapult/subscribers/StateChangeSubscriber.h"
#include "catapult/subscribers/TransactionStatusSubscriber.h"
#include "tests/test/cache/UnsupportedTransactionsChangeSubscribers.h"

namespace catapult { namespace test {

	/// An unsupported state change subscriber.
	class UnsupportedStateChangeSubscriber : public subscribers::StateChangeSubscriber {
	public:
		void notifyScoreChange(const model::ChainScore&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyScoreChange - not supported in mock");
		}

		void notifyStateChange(const consumers::StateChangeInfo&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyStateChange - not supported in mock");
		}
	};

	/// An unsupported transaction status subscriber.
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

	/// An unsupported block change subscriber.
	class UnsupportedBlockChangeSubscriber : public io::BlockChangeSubscriber {
	public:
		void notifyBlock(const model::BlockElement&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyBlock - not supported in mock");
		}

		void notifyDropBlocksAfter(Height) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyDropBlocksAfter - not supported in mock");
		}
	};

	/// An unsupported node subscriber.
	class UnsupportedNodeSubscriber : public subscribers::NodeSubscriber {
	public:
		void notifyNode(const ionet::Node&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyNode - not supported in mock");
		}
	};
}}

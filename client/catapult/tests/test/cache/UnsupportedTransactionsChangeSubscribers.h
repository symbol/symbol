#pragma once
#include "catapult/cache/PtChangeSubscriber.h"
#include "catapult/cache/UtChangeSubscriber.h"

namespace catapult { namespace test {

	// region UnsupportedFlushBehavior / FlushInvoker

	/// Enumeration indicating behavior of unsupported flush operation.
	enum class UnsupportedFlushBehavior {
		/// The operation is ignored.
		Ignore,

		/// The operation throws an exception.
		Throw
	};

	/// Performs specified flush behavior.
	template<UnsupportedFlushBehavior FlushBehavior>
	struct FlushInvoker {
		static void Flush() {
			// do nothing;
		}
	};

	template<>
	struct FlushInvoker<UnsupportedFlushBehavior::Throw> {
		[[noreturn]]
		static void Flush() {
			CATAPULT_THROW_RUNTIME_ERROR("flush - not supported in mock");
		}
	};

	// endregion

	/// An unsupported pt change subscriber.
	template<UnsupportedFlushBehavior FlushBehavior>
	class UnsupportedPtChangeSubscriber : public cache::PtChangeSubscriber {
	public:
		void notifyAddPartials(const TransactionInfos&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyAddPartials - not supported in mock");
		}

		void notifyAddCosignature(const model::TransactionInfo&, const Key&, const Signature&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyAddCosignature - not supported in mock");
		}

		void notifyRemovePartials(const TransactionInfos&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyRemovePartials - not supported in mock");
		}

		void flush() override {
			FlushInvoker<FlushBehavior>::Flush();
		}
	};

	/// An unsupported ut change subscriber
	template<UnsupportedFlushBehavior FlushBehavior>
	class UnsupportedUtChangeSubscriber : public cache::UtChangeSubscriber {
	public:
		void notifyAdds(const TransactionInfos&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyAdds - not supported in mock");
		}

		void notifyRemoves(const TransactionInfos&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyRemoves - not supported in mock");
		}

		void flush() override {
			FlushInvoker<FlushBehavior>::Flush();
		}
	};
}}

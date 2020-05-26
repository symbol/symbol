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
#include "catapult/cache_tx/PtChangeSubscriber.h"
#include "catapult/cache_tx/UtChangeSubscriber.h"

namespace catapult { namespace test {

	// region UnsupportedFlushBehavior / FlushInvoker

	/// Enumeration indicating behavior of unsupported flush operation.
	enum class UnsupportedFlushBehavior {
		/// Operation is ignored.
		Ignore,

		/// Operation throws an exception.
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

	/// Unsupported pt change subscriber.
	template<UnsupportedFlushBehavior FlushBehavior>
	class UnsupportedPtChangeSubscriber : public cache::PtChangeSubscriber {
	public:
		void notifyAddPartials(const TransactionInfos&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyAddPartials - not supported in mock");
		}

		void notifyAddCosignature(const model::TransactionInfo&, const model::Cosignature&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyAddCosignature - not supported in mock");
		}

		void notifyRemovePartials(const TransactionInfos&) override {
			CATAPULT_THROW_RUNTIME_ERROR("notifyRemovePartials - not supported in mock");
		}

		void flush() override {
			FlushInvoker<FlushBehavior>::Flush();
		}
	};

	/// Unsupported ut change subscriber
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

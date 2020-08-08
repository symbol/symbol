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
#include "RoundMessageAggregatorAddResult.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include <memory>
#include <vector>

namespace catapult {
	namespace chain {
		class RoundContext;
		struct RoundMessageAggregatorState;
	}
	namespace model {
		class FinalizationContext;
		struct FinalizationMessage;
	}
}

namespace catapult { namespace chain {

	// region RoundMessageAggregatorView

	/// Read only view on top of round message aggregator.
	class RoundMessageAggregatorView : utils::MoveOnly {
	private:
		using UnknownMessages = std::vector<std::shared_ptr<const model::FinalizationMessage>>;

	public:
		/// Creates a view around \a state with lock context \a readLock.
		RoundMessageAggregatorView(const RoundMessageAggregatorState& state, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Gets the number of messages.
		size_t size() const;

		/// Gets the finalization context.
		const model::FinalizationContext& finalizationContext() const;

		/// Gets the round context.
		const RoundContext& roundContext() const;

		/// Gets a range of short hashes of all messages in the cache.
		/// \note Each short hash consists of the first 4 bytes of the complete hash.
		model::ShortHashRange shortHashes() const;

		/// Gets all finalization messages associated with \a point that do not have a short hash in \a knownShortHashes.
		UnknownMessages unknownMessages(FinalizationPoint point, const utils::ShortHashesSet& knownShortHashes) const;

	private:
		const RoundMessageAggregatorState& m_state;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	// endregion

	// region RoundMessageAggregatorModifier

	/// Write only view on top of round message aggregator.
	class RoundMessageAggregatorModifier : utils::MoveOnly {
	public:
		/// Creates a view around \a state with lock context \a writeLock.
		RoundMessageAggregatorModifier(RoundMessageAggregatorState& state, utils::SpinReaderWriterLock::WriterLockGuard&& writeLock);

	public:
		/// Adds a finalization message (\a pMessage) to the aggregator.
		/// \note Message is a shared_ptr because it is detached from an EntityRange and is kept alive with its associated step.
		RoundMessageAggregatorAddResult add(const std::shared_ptr<model::FinalizationMessage>& pMessage);

	private:
		RoundMessageAggregatorState& m_state;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	// endregion

	// region RoundMessageAggregator

	/// Aggregates finalization messages for a single finalization point.
	class RoundMessageAggregator {
	public:
		/// Creates an aggregator around \a maxResponseSize and \a finalizationContext.
		RoundMessageAggregator(uint64_t maxResponseSize, const model::FinalizationContext& finalizationContext);

		/// Destroys the aggregator.
		~RoundMessageAggregator();

	public:
		/// Gets a read only view of the aggregator.
		RoundMessageAggregatorView view() const;

		/// Gets a write only view of the aggregator.
		RoundMessageAggregatorModifier modifier();

	private:
		std::unique_ptr<RoundMessageAggregatorState> m_pState;
		mutable utils::SpinReaderWriterLock m_lock;
	};

	// endregion
}}

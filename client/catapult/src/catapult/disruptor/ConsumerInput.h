/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "DisruptorTypes.h"
#include "InputSource.h"
#include "catapult/model/AnnotatedEntityRange.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/utils/FileSize.h"

namespace catapult { namespace disruptor {

	/// Consumer input composed of a range of entities augmented with metadata.
	class ConsumerInput {
	public:
		/// Creates a default consumer input.
		ConsumerInput();

		/// Creates a consumer input around a block \a range with an optional input source (\a inputSource).
		explicit ConsumerInput(model::AnnotatedBlockRange&& range, InputSource source = InputSource::Unknown);

		/// Creates a consumer input around a transaction \a range with an optional input source (\a inputSource).
		explicit ConsumerInput(model::AnnotatedTransactionRange&& range, InputSource source = InputSource::Unknown);

	public:
		/// Returns \c true if this input is empty and has no elements.
		bool empty() const;

		/// Returns \c true if this input is not empty and has blocks.
		bool hasBlocks() const;

		/// Returns \c true if this input is not empty and has transactions.
		bool hasTransactions() const;

	public:
		/// Gets the block elements associated with this input.
		BlockElements& blocks();

		/// Gets the const block elements associated with this input.
		const BlockElements& blocks() const;

		/// Gets the (free) transaction elements associated with this input.
		TransactionElements& transactions();

		/// Gets the const (free) transaction elements associated with this input.
		const TransactionElements& transactions() const;

		/// Gets the source of this input.
		InputSource source() const;

		/// Gets the (optional) source identity.
		const model::NodeIdentity& sourceIdentity() const;

		/// Gets the memory size of all elements associated with this input.
		utils::FileSize memorySize() const;

	public:
		/// Detaches the block range associated with this input.
		model::BlockRange detachBlockRange();

		/// Detaches the transaction range associated with this input.
		model::TransactionRange detachTransactionRange();

	public:
		/// Insertion operator for outputting \a input to \a out.
		friend std::ostream& operator<<(std::ostream& out, const ConsumerInput& input);

	private:
		// backing memory
		model::BlockRange m_blockRange;
		model::TransactionRange m_transactionRange;

		// used by consumers
		BlockElements m_blockElements;
		TransactionElements m_transactionElements;

		InputSource m_source;
		model::NodeIdentity m_sourceIdentity;
		utils::FileSize m_memorySize;

		// used by formatting
		Height m_startHeight;
		Height m_endHeight;
	};
}}

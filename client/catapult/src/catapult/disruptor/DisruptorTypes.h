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
#include "catapult/model/Elements.h"
#include <functional>
#include <stdint.h>

namespace catapult { namespace disruptor {

	/// Position within disruptor components.
	using PositionType = uint64_t;

	/// Id of a disruptor element.
	using DisruptorElementId = uint64_t;

	/// Optional code that can provide additional consumer completion information.
	using CompletionCode = uint32_t;

	/// Completion status.
	enum class CompletionStatus : uint8_t {
		/// Processing of the entity was aborted by a consumer.
		Aborted,

		/// Processing of the entity is ongoing.
		Normal,

		/// Processing of the entity was completed and the entity was consumed.
		Consumed
	};

	/// Consumer result severity.
	/// \note Non-success results mean that processing was aborted.
	enum class ConsumerResultSeverity : uint8_t {
		/// Neutral result.
		Neutral,

		/// Success result.
		Success,

		/// Failure result.
		Failure,

		/// Fatal result.
		Fatal
	};

	/// Result of a consumer operation.
	struct ConsumerResult {
	public:
		/// Creates a default result.
		constexpr ConsumerResult() : ConsumerResult(disruptor::CompletionStatus::Normal, 0, ConsumerResultSeverity::Success)
		{}

	private:
		constexpr ConsumerResult(disruptor::CompletionStatus status, disruptor::CompletionCode code, ConsumerResultSeverity severity)
				: CompletionStatus(status)
				, CompletionCode(code)
				, ResultSeverity(severity)
		{}

	public:
		/// Completion status.
		disruptor::CompletionStatus CompletionStatus;

		/// Optional code that can provide additional consumer completion information.
		disruptor::CompletionCode CompletionCode;

		/// Consumer result severity.
		ConsumerResultSeverity ResultSeverity;

	public:
		/// Creates a consumer result indicating that processing should be aborted.
		static constexpr ConsumerResult Abort() {
			return Abort(0, ConsumerResultSeverity::Failure);
		}

		/// Creates a consumer result indicating that processing should be aborted with the specified \a code and \a severity.
		static constexpr ConsumerResult Abort(disruptor::CompletionCode code, ConsumerResultSeverity severity) {
			return ConsumerResult(CompletionStatus::Aborted, code, severity);
		}

		/// Creates a consumer result indicating that processing should continue.
		static constexpr ConsumerResult Continue() {
			return {};
		}

		/// Creates a consumer result indicating that processing has completed with the specified \a code and \a severity.
		static constexpr ConsumerResult Complete(disruptor::CompletionCode code, ConsumerResultSeverity severity) {
			return ConsumerResult(CompletionStatus::Consumed, code, severity);
		}
	};

	/// Extended consumer result passed to completion callback.
	struct ConsumerCompletionResult : public ConsumerResult {
	public:
		/// Creates a default result.
		constexpr ConsumerCompletionResult() : FinalConsumerPosition(std::numeric_limits<PositionType>::max())
		{}

	public:
		/// Position of the final consumer processing the entity.
		PositionType FinalConsumerPosition;
	};

	/// Function signature for signaling that processing finished.
	using ProcessingCompleteFunc = consumer<DisruptorElementId, const ConsumerCompletionResult&>;

	/// Processing element for a transaction unassociated with a block composed of a transaction and metadata.
	struct FreeTransactionElement : public model::TransactionElement {
		/// Creates a transaction element around \a transaction.
		explicit FreeTransactionElement(const model::Transaction& transaction)
				: model::TransactionElement(transaction)
				, ResultSeverity(ConsumerResultSeverity::Success)
		{}

		/// Consumer result severity.
		ConsumerResultSeverity ResultSeverity;
	};

	/// Container of BlockElement.
	using BlockElements = std::vector<model::BlockElement>;

	/// Container of FreeTransactionElement.
	using TransactionElements = std::vector<FreeTransactionElement>;
}}

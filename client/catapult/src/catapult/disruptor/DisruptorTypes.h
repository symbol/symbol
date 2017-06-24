#pragma once
#include "catapult/model/Elements.h"
#include <functional>
#include <stdint.h>

namespace catapult { namespace disruptor {

	/// Position within disruptor components.
	using PositionType = uint64_t;

	/// The id of a disruptor element.
	using DisruptorElementId = uint64_t;

	/// Optional code that can provide additional consumer completion information.
	using CompletionCode = uint32_t;

	/// The completion status.
	enum class CompletionStatus : uint8_t {
		/// The processing of the entity was aborted by a consumer.
		Aborted,
		/// The processing of the entity is ongoing.
		Normal,
		/// The processing of the entity was completed and the entity was consumed.
		Consumed
	};

	/// Result of a consumer operation.
	struct ConsumerResult {
	public:
		/// Creates a default result.
		constexpr ConsumerResult() : ConsumerResult(disruptor::CompletionStatus::Normal, 0)
		{}

	private:
		constexpr ConsumerResult(disruptor::CompletionStatus status, disruptor::CompletionCode code)
				: CompletionStatus(status)
				, CompletionCode(code)
		{}

	public:
		/// The completion status.
		disruptor::CompletionStatus CompletionStatus;

		/// Optional code that can provide additional consumer completion information.
		disruptor::CompletionCode CompletionCode;

	public:
		/// Creates a consumer result indicating that processing should be aborted.
		static constexpr ConsumerResult Abort() {
			return Abort(0);
		}

		/// Creates a consumer result indicating that processing should be aborted with the specified \a code.
		static constexpr ConsumerResult Abort(disruptor::CompletionCode code) {
			return ConsumerResult(CompletionStatus::Aborted, code);
		}

		/// Creates a consumer result indicating that processing should continue.
		static constexpr ConsumerResult Continue() {
			return {};
		}

		/// Creates a consumer result indicating that processing has completed.
		static constexpr ConsumerResult Complete() {
			return ConsumerResult(CompletionStatus::Consumed, 0);
		}
	};

	/// Extended consumer result passed to completion callback.
	struct ConsumerCompletionResult : public ConsumerResult {
	public:
		/// Creates a default result.
		constexpr ConsumerCompletionResult() : FinalConsumerPosition(std::numeric_limits<PositionType>::max())
		{}

	public:
		/// The position of the final consumer processing the entity.
		PositionType FinalConsumerPosition;
	};

	/// Function signature for signaling that processing finished.
	using ProcessingCompleteFunc = std::function<void (DisruptorElementId, const ConsumerCompletionResult&)>;

	/// Processing element for a transaction unassociated with a block composed of a transaction and metadata.
	struct FreeTransactionElement : public model::TransactionElement {
		/// Creates a transaction element around \a transaction.
		explicit FreeTransactionElement(const model::Transaction& transaction)
				: model::TransactionElement(transaction)
				, Skip(false)
		{}

		/// \c true if the element should be skipped.
		bool Skip;
	};

	/// A container of BlockElement.
	using BlockElements = std::vector<model::BlockElement>;

	/// A container of FreeTransactionElement.
	using TransactionElements = std::vector<FreeTransactionElement>;
}}

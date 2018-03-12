#pragma once
#include "ConsumerInput.h"
#include "catapult/utils/SpinLock.h"

namespace catapult { namespace disruptor {

	/// Augments consumer input with disruptor metadata.
	class DisruptorElement {
	public:
		/// Creates a default disruptor element.
		DisruptorElement()
				: m_id(static_cast<uint64_t>(-1))
				, m_processingComplete([](auto, auto) {})
				, m_pSpinLock(std::make_unique<utils::SpinLock>())
		{}

		/// Creates a disruptor element around \a input with \a id and a completion handler \a processingComplete.
		explicit DisruptorElement(ConsumerInput&& input, DisruptorElementId id, const ProcessingCompleteFunc& processingComplete)
				: m_input(std::move(input))
				, m_id(id)
				, m_processingComplete(processingComplete)
				, m_pSpinLock(std::make_unique<utils::SpinLock>())
		{}

	public:
		/// Gets the consumer input.
		ConsumerInput& input() {
			return m_input;
		}

		/// Gets the consumer input.
		const ConsumerInput& input() const {
			return m_input;
		}

		/// Gets the element id.
		DisruptorElementId id() const {
			return m_id;
		}

		/// Returns \c true if the element is skipped.
		bool isSkipped() const {
			utils::SpinLockGuard guard(*m_pSpinLock);
			return CompletionStatus::Aborted == m_result.CompletionStatus;
		}

		/// Gets the current element completion result.
		ConsumerCompletionResult completionResult() const {
			utils::SpinLockGuard guard(*m_pSpinLock);
			return m_result;
		}

	public:
		/// Marks the element as skipped at \a position with \a code.
		void markSkipped(PositionType position, CompletionCode code) {
			utils::SpinLockGuard guard(*m_pSpinLock);
			m_result.CompletionStatus = CompletionStatus::Aborted;
			m_result.CompletionCode = code;
			m_result.FinalConsumerPosition = position;
		}

		/// Calls the completion handler for the element.
		void markProcessingComplete() {
			m_processingComplete(m_id, m_result);
		}

	private:
		ConsumerInput m_input;
		DisruptorElementId m_id;
		ProcessingCompleteFunc m_processingComplete;
		ConsumerCompletionResult m_result;
		std::unique_ptr<utils::SpinLock> m_pSpinLock; // unique_ptr to allow moving of element
	};

	/// Insertion operator for outputting \a element to \a out.
	std::ostream& operator<<(std::ostream& out, const DisruptorElement& element);

	/// Returns \c true if \a id matches \a interval.
	constexpr bool IsIntervalElementId(DisruptorElementId id, size_t interval) {
		return 0 != interval && 0 == id % interval;
	}
}}

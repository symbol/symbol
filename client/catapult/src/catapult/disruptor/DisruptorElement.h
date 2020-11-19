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
		DisruptorElement(ConsumerInput&& input, DisruptorElementId id, const ProcessingCompleteFunc& processingComplete)
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
		/// Marks the element as skipped at \a position with \a result.
		void markSkipped(PositionType position, const ConsumerResult& result) {
			utils::SpinLockGuard guard(*m_pSpinLock);
			m_result.CompletionStatus = CompletionStatus::Aborted;
			m_result.CompletionCode = result.CompletionCode;
			m_result.ResultSeverity = result.ResultSeverity;
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

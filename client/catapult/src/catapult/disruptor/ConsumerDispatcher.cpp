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

#include "ConsumerDispatcher.h"
#include "ConsumerEntry.h"
#include "catapult/thread/ThreadInfo.h"
#include "catapult/utils/Functional.h"
#include <thread>

namespace catapult { namespace disruptor {

	namespace {
		const ConsumerDispatcherOptions& CheckOptions(const ConsumerDispatcherOptions& options) {
			if (!options.DispatcherName || 0 == options.DisruptorSlotCount || utils::FileSize() == options.DisruptorMaxMemorySize)
				CATAPULT_THROW_INVALID_ARGUMENT("consumer dispatcher options are invalid");

			return options;
		}

		void LogCompletion(const DisruptorElement& element, const DisruptorBarriers& barriers, size_t elementTraceInterval) {
			if (!IsIntervalElementId(element.id(), elementTraceInterval))
				return;

			auto minPosition = barriers[barriers.size() - 1].position();
			auto maxPosition = barriers[0].position();
			CATAPULT_LOG(info)
					<< "completing processing of " << element
					<< ", last consumer is " << (maxPosition - minPosition) << " elements behind";
		}
	}

	ConsumerDispatcher::ConsumerDispatcher(const ConsumerDispatcherOptions& options, const std::vector<DisruptorConsumer>& consumers)
			: ConsumerDispatcher(options, consumers, [](const auto&, const auto&) {})
	{}

	ConsumerDispatcher::ConsumerDispatcher(
			const ConsumerDispatcherOptions& options,
			const std::vector<DisruptorConsumer>& consumers,
			const DisruptorInspector& inspector)
			: NamedObjectMixin(CheckOptions(options).DispatcherName)
			, m_options(options)
			, m_keepRunning(true)
			, m_barriers(consumers.size() + 1)
			, m_disruptor(m_options.DisruptorSlotCount, m_options.ElementTraceInterval)
			, m_inspector(inspector)
			, m_numActiveElements(0)
			, m_memorySize(0) {
		auto currentLevel = 0u;
		for (const auto& consumer : consumers) {
			ConsumerEntry consumerEntry(currentLevel++);
			m_threads.spawn([pThis = this, consumerEntry, consumer]() mutable {
				thread::SetThreadName(std::to_string(consumerEntry.level()) + " " + pThis->name());
				while (pThis->m_keepRunning) {
					auto* pDisruptorElement = pThis->tryNext(consumerEntry);
					if (!pDisruptorElement) {
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						continue;
					}

					auto result = consumer(pDisruptorElement->input());
					if (CompletionStatus::Aborted == result.CompletionStatus)
						pThis->m_disruptor.markSkipped(consumerEntry.position(), result);

					pThis->advance(consumerEntry);
				}
			});
		}

		CATAPULT_LOG(info) << m_options.DispatcherName << " ConsumerDispatcher spawned " << m_threads.size() << " workers";
	}

	ConsumerDispatcher::~ConsumerDispatcher() {
		shutdown();
	}

	void ConsumerDispatcher::shutdown() {
		m_keepRunning = false;
		m_threads.join();
	}

	bool ConsumerDispatcher::isRunning() const {
		return m_keepRunning.load();
	}

	size_t ConsumerDispatcher::size() const {
		return m_threads.size();
	}

	size_t ConsumerDispatcher::numAddedElements() const {
		return m_disruptor.added();
	}

	size_t ConsumerDispatcher::numActiveElements() const {
		return m_numActiveElements.load();
	}

	utils::FileSize ConsumerDispatcher::memorySize() const {
		return utils::FileSize::FromBytes(m_memorySize.load());
	}

	DisruptorElement* ConsumerDispatcher::tryNext(ConsumerEntry& consumerEntry) {
		while (true) {
			auto consumerBarrierPosition = m_barriers[consumerEntry.level()].position();
			auto consumerPosition = consumerEntry.position();
			if (consumerPosition == consumerBarrierPosition)
				return nullptr;

			if (!m_disruptor.isSkipped(consumerPosition))
				return &m_disruptor.elementAt(consumerPosition);

			advance(consumerEntry);
		}
	}

	void ConsumerDispatcher::advance(ConsumerEntry& consumerEntry) {
		auto consumerPosition = consumerEntry.position();
		consumerEntry.advance();
		m_barriers[consumerEntry.level() + 1].advance();

		// if advance was called by the last consumer, then run the inspector on the (current) thread of the last consumer
		if (consumerEntry.level() + 1 != m_barriers.size() - 1)
			return;

		auto& element = m_disruptor.elementAt(consumerPosition);
		LogCompletion(element, m_barriers, m_options.ElementTraceInterval);
		m_inspector(element.input(), element.completionResult());
		element.markProcessingComplete();
	}

	bool ConsumerDispatcher::canProcessNextElement() const {
		auto minPosition = m_barriers[m_barriers.size() - 1].position();
		auto maxPosition = m_barriers[0].position();
		auto requiredCapacity = maxPosition - minPosition + 1 + 1; // check for space for *next* element
		auto totalCapacity = m_disruptor.capacity();

		if (requiredCapacity < totalCapacity)
			return true;

		CATAPULT_LOG(warning) << "disruptor is full (minPosition = " << minPosition << ", maxPosition = " << maxPosition << ")";
		return requiredCapacity == totalCapacity;
	}

	ProcessingCompleteFunc ConsumerDispatcher::wrap(const ProcessingCompleteFunc& processingComplete, utils::FileSize inputMemorySize) {
		return [this, processingComplete, inputMemorySize](auto elementId, const auto& result) {
			processingComplete(elementId, result);

			m_memorySize -= inputMemorySize.bytes();
			--m_numActiveElements;
		};
	}

	DisruptorElementId ConsumerDispatcher::processElement(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete) {
		if (input.empty()) {
			CATAPULT_LOG(trace) << "dispatcher is ignoring empty input (" << input << ")";
			return 0;
		}

		auto inputMemorySize = input.memorySize();

		// need to atomically check spare capacity AND add element
		utils::SpinLockGuard guard(m_addSpinLock);
		auto isFull = !canProcessNextElement();
		if (m_options.DisruptorMaxMemorySize.bytes() - m_memorySize < inputMemorySize.bytes()) {
			CATAPULT_LOG(warning)
					<< "disruptor memory is full (max = " << m_options.DisruptorMaxMemorySize
					<< ", current = " << utils::FileSize::FromBytes(m_memorySize) << ")";
			isFull = true;
		}

		if (isFull) {
			if (m_options.ShouldThrowWhenFull)
				CATAPULT_THROW_RUNTIME_ERROR("consumer is too far behind");

			return 0;
		}

		m_memorySize += inputMemorySize.bytes();
		++m_numActiveElements;

		auto id = m_disruptor.add(std::move(input), wrap(processingComplete, inputMemorySize));
		m_barriers[0].advance();
		return id;
	}

	DisruptorElementId ConsumerDispatcher::processElement(ConsumerInput&& input) {
		return processElement(std::move(input), [](auto, auto) {});
	}
}}

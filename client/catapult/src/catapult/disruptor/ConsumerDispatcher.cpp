#include "ConsumerDispatcher.h"
#include "ConsumerEntry.h"
#include "catapult/thread/ThreadInfo.h"
#include "catapult/utils/ExceptionLogging.h"
#include "catapult/utils/Functional.h"
#include <thread>

namespace catapult { namespace disruptor {

	namespace {
		const ConsumerDispatcherOptions& CheckOptions(const ConsumerDispatcherOptions& options) {
			if (nullptr == options.DispatcherName || 0 == options.DisruptorSize)
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
			, m_elementTraceInterval(options.ElementTraceInterval)
			, m_shouldThrowIfFull(options.ShouldThrowIfFull)
			, m_keepRunning(true)
			, m_barriers(consumers.size() + 1)
			, m_disruptor(options.DisruptorSize, options.ElementTraceInterval)
			, m_inspector(inspector)
			, m_numActiveElements(0) {
		auto currentLevel = 0u;
		for (const auto& consumer : consumers) {
			ConsumerEntry consumerEntry(currentLevel++);
			m_threads.create_thread([pThis = this, consumerEntry, consumer]() mutable {
				thread::SetThreadName(std::to_string(consumerEntry.level()) + " " + pThis->name());
				while (pThis->m_keepRunning) {
					try {
						auto* pDisruptorElement = pThis->tryNext(consumerEntry);
						if (!pDisruptorElement) {
							std::this_thread::sleep_for(std::chrono::milliseconds(10));
							continue;
						}

						auto result = consumer(pDisruptorElement->input());
						if (CompletionStatus::Aborted == result.CompletionStatus)
							pThis->m_disruptor.markSkipped(consumerEntry.position(), result.CompletionCode);

						pThis->advance(consumerEntry);
					} catch (...) {
						CATAPULT_LOG(fatal)
								<< "consumer at level " << consumerEntry.level() << " threw exception: "
								<< EXCEPTION_DIAGNOSTIC_MESSAGE();
						utils::CatapultLogFlush();
						throw;
					}
				}
			});
		}

		CATAPULT_LOG(info) << options.DispatcherName << " ConsumerDispatcher spawned " << m_threads.size() << " workers";
	}

	ConsumerDispatcher::~ConsumerDispatcher() {
		shutdown();
	}

	void ConsumerDispatcher::shutdown() {
		m_keepRunning = false;
		m_threads.join_all();
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
		LogCompletion(element, m_barriers, m_elementTraceInterval);
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

	ProcessingCompleteFunc ConsumerDispatcher::wrap(const ProcessingCompleteFunc& processingComplete) {
		return [processingComplete, &numActiveElements = m_numActiveElements](auto elementId, const auto& result) {
			processingComplete(elementId, result);
			--numActiveElements;
		};
	}

	DisruptorElementId ConsumerDispatcher::processElement(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete) {
		if (input.empty()) {
			CATAPULT_LOG(trace) << "dispatcher is ignoring empty input (" << input << ")";
			return 0;
		}

		// need to atomically check spare capacity AND add element
		utils::SpinLockGuard guard(m_addSpinLock);
		if (!canProcessNextElement()) {
			if (m_shouldThrowIfFull)
				CATAPULT_THROW_RUNTIME_ERROR("consumer is too far behind");

			return 0;
		}

		++m_numActiveElements;
		auto id = m_disruptor.add(std::move(input), wrap(processingComplete));
		m_barriers[0].advance();
		return id;
	}

	DisruptorElementId ConsumerDispatcher::processElement(ConsumerInput&& input) {
		return processElement(std::move(input), [](auto, auto) {});
	}
}}

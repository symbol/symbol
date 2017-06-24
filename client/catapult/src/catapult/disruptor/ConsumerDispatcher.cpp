#include "ConsumerDispatcher.h"
#include "ConsumerEntry.h"
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
			CATAPULT_LOG(debug)
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
			, m_keepRunning(true)
			, m_barriers(consumers.size() + 1)
			, m_disruptor(options.DisruptorSize, options.ElementTraceInterval)
			, m_inspector(inspector) {
		auto currentLevel = 0u;
		for (const auto& consumer : consumers) {
			ConsumerEntry consumerEntry(currentLevel++);
			m_threads.create_thread([pThis = this, consumerEntry, consumer]() mutable {
				while (pThis->m_keepRunning) {
					try {
						auto pDisruptorElement = pThis->tryNext(consumerEntry);
						if (!pDisruptorElement) {
							std::this_thread::sleep_for(std::chrono::milliseconds(10));
							continue;
						}

						auto result = consumer(pDisruptorElement->input());
						if (CompletionStatus::Aborted == result.CompletionStatus)
							pThis->m_disruptor.markSkipped(consumerEntry.position(), result.CompletionCode);

						pThis->advance(consumerEntry);
					} catch (...) {
						CATAPULT_LOG(fatal) << "consumer at level " << consumerEntry.level() << " threw exception: "
								<< boost::current_exception_diagnostic_information();
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

	void ConsumerDispatcher::checkCapacity(const ConsumerEntry& consumerEntry) const {
		auto minPosition = m_barriers[m_barriers.size() - 1].position();
		auto maxPosition = m_barriers[0].position();
		auto requiredCapacity = maxPosition - minPosition + 1;
		auto totalCapacity = m_disruptor.capacity();

		if (requiredCapacity < totalCapacity)
			return;

		CATAPULT_LOG(warning) << "disruptor is full (minPosition = " << minPosition << ", maxPosition = " << maxPosition
				<< ") at consumer " << consumerEntry.level();

		if (requiredCapacity > totalCapacity)
			CATAPULT_THROW_RUNTIME_ERROR("consumer is too far behind");
	}

	DisruptorElement* ConsumerDispatcher::tryNext(ConsumerEntry& consumerEntry) {
		while (true) {
			auto consumerBarrierPosition = m_barriers[consumerEntry.level()].position();
			auto consumerPosition = consumerEntry.position();
			if (consumerPosition == consumerBarrierPosition)
				return nullptr;

			checkCapacity(consumerEntry);

			if (!m_disruptor.isSkipped(consumerPosition))
				return &m_disruptor.elementAt(consumerPosition);

			advance(consumerEntry);
		}
	}

	void ConsumerDispatcher::advance(ConsumerEntry& consumerEntry) {
		auto consumerPosition = consumerEntry.position();
		consumerEntry.advance();
		m_barriers[consumerEntry.level() + 1].advance();

		// If it's advance in the last consumer run the inspector.
		// Inspector runs within a thread of the last consumer.
		if (consumerEntry.level() + 1 != m_barriers.size() - 1)
			return;

		auto& element = m_disruptor.elementAt(consumerPosition);
		LogCompletion(element, m_barriers, m_elementTraceInterval);
		m_inspector(element.input(), element.completionResult());
		element.markProcessingComplete();
	}

	DisruptorElementId ConsumerDispatcher::processElement(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete) {
		if (input.empty()) {
			CATAPULT_LOG(trace) << "dispatcher is ignoring empty input (" << input << ")";
			return 0u;
		}

		auto id = m_disruptor.add(std::move(input), processingComplete);
		m_barriers[0].advance();
		return id;
	}

	DisruptorElementId ConsumerDispatcher::processElement(ConsumerInput&& input) {
		return processElement(std::move(input), [](auto, auto) {});
	}
}}

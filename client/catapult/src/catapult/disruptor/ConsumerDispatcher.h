#pragma once
#include "Disruptor.h"
#include "DisruptorConsumer.h"
#include "DisruptorInspector.h"
#include "catapult/utils/NamedObject.h"
#include <boost/thread.hpp>
#include <atomic>

namespace catapult { namespace disruptor { class ConsumerEntry; } }

namespace catapult { namespace disruptor {

	/// Consumer dispatcher options.
	struct ConsumerDispatcherOptions {
	public:
		/// Creates options around \a dispatcherName and \a disruptorSize.
		constexpr ConsumerDispatcherOptions(const char* dispatcherName, size_t disruptorSize)
				: ConsumerDispatcherOptions(dispatcherName, disruptorSize, 1)
		{}

		/// Creates options around \a dispatcherName, \a disruptorSize and \a elementTraceInterval.
		constexpr ConsumerDispatcherOptions(const char* dispatcherName, size_t disruptorSize, size_t elementTraceInterval)
				: DispatcherName(dispatcherName)
				, DisruptorSize(disruptorSize)
				, ElementTraceInterval(elementTraceInterval)
		{}

	public:
		/// The name of the dispatcher.
		const char* DispatcherName;

		/// The disruptor size.
		size_t DisruptorSize;

		/// The multiple of elements at which an element should be traced through queue and completion.
		size_t ElementTraceInterval;
	};

	/// Dispatcher for disruptor consumers.
	class ConsumerDispatcher final : public utils::NamedObjectMixin {
	public:
		/// Creates a dispatcher of \a consumers configured with \a options.
		/// Inspector (\a inspector) is a special consumer that is always run (independent of skip) and as a last one.
		/// Inspector runs within a thread of the last consumer.
		ConsumerDispatcher(
			const ConsumerDispatcherOptions& options,
			const std::vector<DisruptorConsumer>& consumers,
			const DisruptorInspector& inspector);

		/// Creates a dispatcher of \a consumers configured with \a options.
		explicit ConsumerDispatcher(
			const ConsumerDispatcherOptions& options,
			const std::vector<DisruptorConsumer>& consumers);

		~ConsumerDispatcher();

	public:
		/// Shuts down dispatcher and stops all threads.
		void shutdown();

		/// Returns \c true if dispatcher is running, \c false otherwise.
		bool isRunning() const;

		/// Returns the number of registered consumers.
		size_t size() const;

		/// Pushes the \a input into underlying disruptor and returns the assigned element id.
		/// Once the processing of the input is complete, \a processingComplete will be called.
		DisruptorElementId processElement(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete);

		/// Pushes the \a input into underlying disruptor and returns the assigned element id.
		DisruptorElementId processElement(ConsumerInput&& input);

		/// Returns the total number of elements added to the disruptor.
		size_t numAddedElements() const;

	private:
		void checkCapacity(const ConsumerEntry& consumerEntry) const;

		DisruptorElement* tryNext(ConsumerEntry& consumerEntry);

		void advance(ConsumerEntry& consumerEntry);

	private:
		size_t m_elementTraceInterval;
		std::atomic_bool m_keepRunning;
		DisruptorBarriers m_barriers;
		Disruptor m_disruptor;
		DisruptorInspector m_inspector;
		boost::thread_group m_threads;
	};
}}

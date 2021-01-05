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
#include "ConsumerDispatcherOptions.h"
#include "Disruptor.h"
#include "DisruptorConsumer.h"
#include "DisruptorInspector.h"
#include "catapult/thread/ThreadGroup.h"
#include "catapult/utils/NamedObject.h"
#include <atomic>

namespace catapult { namespace disruptor { class ConsumerEntry; } }

namespace catapult { namespace disruptor {

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
		ConsumerDispatcher(const ConsumerDispatcherOptions& options, const std::vector<DisruptorConsumer>& consumers);

		~ConsumerDispatcher();

	public:
		/// Shuts down dispatcher and stops all threads.
		void shutdown();

		/// Returns \c true if dispatcher is running, \c false otherwise.
		bool isRunning() const;

		/// Gets the number of registered consumers.
		size_t size() const;

		/// Pushes the \a input into underlying disruptor and returns the assigned element id.
		/// Once the processing of the input is complete, \a processingComplete will be called.
		DisruptorElementId processElement(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete);

		/// Pushes the \a input into underlying disruptor and returns the assigned element id.
		DisruptorElementId processElement(ConsumerInput&& input);

		/// Gets the total number of elements added to the disruptor.
		size_t numAddedElements() const;

		/// Gets the number of elements currently in the disruptor.
		size_t numActiveElements() const;

		/// Gets the cumulative size of all elements currently in the disruptor.
		utils::FileSize memorySize() const;

	private:
		DisruptorElement* tryNext(ConsumerEntry& consumerEntry);

		void advance(ConsumerEntry& consumerEntry);

		bool canProcessNextElement() const;

		ProcessingCompleteFunc wrap(const ProcessingCompleteFunc& processingComplete, utils::FileSize inputMemorySize);

	private:
		ConsumerDispatcherOptions m_options;
		std::atomic_bool m_keepRunning;
		DisruptorBarriers m_barriers;
		Disruptor m_disruptor;
		DisruptorInspector m_inspector;
		thread::ThreadGroup m_threads;
		std::atomic<size_t> m_numActiveElements;
		std::atomic<uint64_t> m_memorySize;

		utils::SpinLock m_addSpinLock; // lock to serialize access to Disruptor::add
	};
}}

#include "Disruptor.h"
#include "catapult/utils/Functional.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"
#include <thread>

namespace catapult { namespace disruptor {

	// short rationale for lack of locks:
	//  1. m_container is initialized with size, so most operation here don't require locks
	//     under assumption that "consumers are fast enough", even if not,
	//     there's a check and a throw inside consumer dispatcher.
	//  2. add() is guarded by a spinlock
	//  3. pSkip inside DisruptorElement is atomic

	Disruptor::Disruptor(size_t disruptorSize, size_t elementTraceInterval)
			: m_elementTraceInterval(elementTraceInterval)
			, m_container(disruptorSize)
			, m_allElementsCount(0)
	{}

	DisruptorElementId Disruptor::add(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete) {
		auto element = DisruptorElement(std::move(input), ++m_allElementsCount, processingComplete);
		if (IsIntervalElementId(element.id(), m_elementTraceInterval))
			CATAPULT_LOG(debug) << "disruptor queuing " << element;

		SpinLockGuard guard(m_containerSpinLock);
		m_container.push_back(std::move(element));
		return element.id();
	}

	void Disruptor::markSkipped(PositionType position, CompletionCode code) {
		m_container[position].markSkipped(position, code);
	}

	bool Disruptor::isSkipped(PositionType position) const {
		return m_container[position].isSkipped();
	}
}}

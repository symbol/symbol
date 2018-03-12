#include "Disruptor.h"
#include "catapult/utils/Functional.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"

namespace catapult { namespace disruptor {

	// short rationale for lack of locks:
	//  1. m_container is initialized with size, so most operations here don't require locks
	//  2. add is guarded by a lock inside ConsumerDispatcher, which checks if the Disruptor is full
	//  3. markSkipped and isSkipped are guarded by a lock inside DisruptorElement

	Disruptor::Disruptor(size_t disruptorSize, size_t elementTraceInterval)
			: m_elementTraceInterval(elementTraceInterval)
			, m_container(disruptorSize)
			, m_allElementsCount(0)
	{}

	DisruptorElementId Disruptor::add(ConsumerInput&& input, const ProcessingCompleteFunc& processingComplete) {
		auto element = DisruptorElement(std::move(input), ++m_allElementsCount, processingComplete);
		if (IsIntervalElementId(element.id(), m_elementTraceInterval))
			CATAPULT_LOG(debug) << "disruptor queuing " << element;

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

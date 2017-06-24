#include "LocalNodeStateListeners.h"

namespace catapult { namespace local { namespace api {

	void LocalNodeStateListeners::notifyStateChange(
			const model::ChainScore& chainScore,
			const consumers::StateChangeInfo& stateChangeInfo) const {
		if (m_stateChangeListener)
			m_stateChangeListener(chainScore, stateChangeInfo);
	}

	void LocalNodeStateListeners::subscribeStateChange(const StateChangeListener& stateChangeListener) {
		m_stateChangeListener = stateChangeListener;
	}
}}}

#pragma once
#include "catapult/local/LocalNodeStateRef.h"
#include <functional>

namespace catapult {
	namespace consumers { struct StateChangeInfo; }
	namespace model { class ChainScore; }
}

namespace catapult { namespace local { namespace api {

	using StateChangeListener = std::function<void (const model::ChainScore&, const consumers::StateChangeInfo&)>;

	/// Wraps listeners for api state change notifications.
	class LocalNodeStateListeners {
	public:
		/// Send notification composed of new \a chainScore and state change (\a stateChangeInfo) to listener.
		void notifyStateChange(const model::ChainScore& chainScore, const consumers::StateChangeInfo& stateChangeInfo) const;

		/// Register state change listener (\a stateChangeListener).
		void subscribeStateChange(const StateChangeListener& stateChangeListener);

	private:
		StateChangeListener m_stateChangeListener;
	};
}}}

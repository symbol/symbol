#pragma once
#include "LocalNodeStateListeners.h"
#include "catapult/local/LocalNodeStateRef.h"

namespace catapult { namespace local { namespace api {

	/// Wraps node state and listeners.
	class LocalNodeApiState {
	public:
		/// Creates local node api state around local node state (\a stateRef).
		explicit LocalNodeApiState(const LocalNodeStateRef& stateRef) : m_state(stateRef)
		{}

		/// Returns reference to state.
		const LocalNodeStateRef& stateRef() const {
			return m_state;
		}

		/// Returns reference to listeners.
		const LocalNodeStateListeners& listeners() const {
			return m_listeners;
		}

		/// Sets state change listener (\a stateChangeListener).
		void subscribeStateChange(const StateChangeListener& stateChangeListener) {
			m_listeners.subscribeStateChange(stateChangeListener);
		}

	private:
		LocalNodeStateRef m_state;
		LocalNodeStateListeners m_listeners;
	};
}}}

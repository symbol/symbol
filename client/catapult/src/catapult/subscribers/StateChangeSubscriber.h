#pragma once

namespace catapult {
	namespace consumers { struct StateChangeInfo; }
	namespace model { class ChainScore; }
}

namespace catapult { namespace subscribers {

	/// State change subscriber.
	class StateChangeSubscriber {
	public:
		virtual ~StateChangeSubscriber() {}

	public:
		/// Indicates chain score was changed to \a chainScore.
		virtual void notifyScoreChange(const model::ChainScore& chainScore) = 0;

		/// Indicates state was changed with change information in \a stateChangeInfo.
		virtual void notifyStateChange(const consumers::StateChangeInfo& stateChangeInfo) = 0;
	};
}}

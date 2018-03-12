#pragma once
#include "extensions/mongo/src/ChainScoreProvider.h"
#include "catapult/model/ChainScore.h"

namespace catapult { namespace mocks {

	/// A mock chain score provider.
	class MockChainScoreProvider : public mongo::ChainScoreProvider {
	public:
		void saveScore(const model::ChainScore&) override {
			CATAPULT_THROW_RUNTIME_ERROR("not implemented");
		}

		model::ChainScore loadScore() const override {
			return model::ChainScore();
		}
	};
}}

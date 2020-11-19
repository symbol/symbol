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

#include "StateHashCalculator.h"
#include "catapult/model/Block.h"
#include "tests/int/node/test/LocalNodeNemesisHashTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"

namespace catapult { namespace test {

	StateHashCalculator::StateHashCalculator()
			: m_stateVerificationMode(StateVerificationMode::Disabled)
			, m_config(CreateUninitializedCatapultConfiguration())
			, m_catapultCache({})
			, m_isDirty(false)
	{}

	StateHashCalculator::StateHashCalculator(const config::CatapultConfiguration& config)
			: m_stateVerificationMode(StateVerificationMode::Enabled)
			, m_config(config)
			, m_pPluginManager(CreatePluginManagerWithRealPlugins(m_config))
			, m_catapultCache(m_pPluginManager->createCache())
			, m_isDirty(false)
	{}

	const std::string& StateHashCalculator::dataDirectory() const {
		return m_config.User.DataDirectory;
	}

	const config::CatapultConfiguration& StateHashCalculator::config() const {
		return m_config;
	}

	Hash256 StateHashCalculator::execute(const model::Block& block) {
		if (StateVerificationMode::Disabled == m_stateVerificationMode || m_isDirty)
			return Hash256();

		Hash256 blockStateHash;
		auto cacheDelta = m_catapultCache.createDelta();
		try {
			blockStateHash = CalculateBlockStateHash(block, cacheDelta, *m_pPluginManager);
		} catch (const catapult_runtime_error&) {
			// if state is invalid (e.g. negative balance), zero out state hash and bypass subsequent state hash calculations
			CATAPULT_LOG(debug)
					<< "block state hash calculation failed at height " << block.Height
					<< ", marking block and remaining chain as dirty";
			blockStateHash = Hash256();
			m_isDirty = true;
		}

		m_catapultCache.commit(block.Height);

		return blockStateHash;
	}

	void StateHashCalculator::updateStateHash(model::Block& block) {
		block.StateHash = execute(block);
	}
}}

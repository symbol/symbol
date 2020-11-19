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
#include "catapult/cache/CatapultCache.h"
#include "catapult/config/CatapultConfiguration.h"

namespace catapult {
	namespace model { struct Block; }
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace test {

	/// Calculates state hashes by executing blocks.
	class StateHashCalculator {
	private:
		enum class StateVerificationMode { Enabled, Disabled };

	public:
		/// Creates a default calculator that does not calculate state hashes.
		StateHashCalculator();

		/// Creates a calculator around \a config that calculates state hashes.
		explicit StateHashCalculator(const config::CatapultConfiguration& config);

	public:
		/// Gets the data directory where intermediate data is output.
		const std::string& dataDirectory() const;

		/// Gets the configuration.
		const config::CatapultConfiguration& config() const;

	public:
		/// Executes \a block and returns the block state hash.
		Hash256 execute(const model::Block& block);

		/// Executes \a block and updates its block state hash.
		void updateStateHash(model::Block& block);

	private:
		StateVerificationMode m_stateVerificationMode;
		config::CatapultConfiguration m_config;

		std::shared_ptr<plugins::PluginManager> m_pPluginManager;
		cache::CatapultCache m_catapultCache;
		bool m_isDirty;
	};
}}

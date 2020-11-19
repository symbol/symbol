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
#include "catapult/extensions/LocalNodeStateRef.h"
#include <memory>
#include <string>

namespace catapult { namespace model { struct BlockChainConfiguration; } }

namespace catapult { namespace test {

	/// Local node test state.
	class LocalNodeTestState {
	public:
		/// Creates default state.
		LocalNodeTestState();

		/// Creates default state around \a config.
		explicit LocalNodeTestState(const model::BlockChainConfiguration& config);

		/// Creates default state around \a cache.
		explicit LocalNodeTestState(cache::CatapultCache&& cache);

		/// Creates default state around \a config, \a userDataDirectory and \a cache.
		LocalNodeTestState(
				const model::BlockChainConfiguration& config,
				const std::string& userDataDirectory,
				cache::CatapultCache&& cache);

		/// Destroys the state.
		~LocalNodeTestState();

	public:
		/// Gets a state ref.
		extensions::LocalNodeStateRef ref();

	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}

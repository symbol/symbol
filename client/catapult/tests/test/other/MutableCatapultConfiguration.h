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
#include "catapult/config/CatapultConfiguration.h"

namespace catapult { namespace test {

	/// Comprehensive mutable configuration for a catapult process.
	class MutableCatapultConfiguration {
	public:
		/// Creates a mutable catapult configuration.
		MutableCatapultConfiguration()
				: BlockChain(model::BlockChainConfiguration::Uninitialized())
				, Node(config::NodeConfiguration::Uninitialized())
				, Logging(config::LoggingConfiguration::Uninitialized())
				, User(config::UserConfiguration::Uninitialized())
				, Extensions(config::ExtensionsConfiguration::Uninitialized())
				, Inflation(config::InflationConfiguration::Uninitialized())
		{}

	public:
		/// Block chain configuration.
		model::BlockChainConfiguration BlockChain;

		/// Node configuration.
		config::NodeConfiguration Node;

		/// Logging configuration.
		config::LoggingConfiguration Logging;

		/// User configuration.
		config::UserConfiguration User;

		/// Extensions configuration.
		config::ExtensionsConfiguration Extensions;

		/// Inflation configuration.
		config::InflationConfiguration Inflation;

	public:
		/// Converts this mutable configuration to a const configuration.
		config::CatapultConfiguration ToConst() {
			return config::CatapultConfiguration(
					std::move(BlockChain),
					std::move(Node),
					std::move(Logging),
					std::move(User),
					std::move(Extensions),
					std::move(Inflation));
		}
	};
}}

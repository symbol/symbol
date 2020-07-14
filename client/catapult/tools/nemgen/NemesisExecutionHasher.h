/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "blockhashes/NemesisBlockHashesCalculator.h"
#include <string>

namespace catapult { namespace config { class CatapultConfiguration; } }

namespace catapult { namespace tools { namespace nemgen {

	/// Nemesis block execution dependent hashes information.
	struct NemesisExecutionHashesDescriptor : public BlockExecutionHashesInfo {
		/// Textual summary including sub cache hashes.
		std::string Summary;
	};

	/// Calculates and logs the nemesis block execution dependent hashes after executing nemesis \a blockElement
	/// for network configured with \a config and \a pluginManager.
	NemesisExecutionHashesDescriptor CalculateAndLogNemesisExecutionHashes(
			const model::BlockElement& blockElement,
			const config::CatapultConfiguration& config,
			plugins::PluginManager& pluginManager);
}}}

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
#include "statehash/StateHashCalculator.h"

namespace catapult { namespace tools { namespace nemgen {

	/// Possible cache database cleanup modes.
	enum class CacheDatabaseCleanupMode {
		/// Perform no cleanup.
		None,
		/// Purge after execution.
		Purge
	};

	/// Nemesis state hash information.
	struct NemesisStateHashDescriptor {
		/// State hash.
		Hash256 Hash;
		/// Textual summary including subcache hashes.
		std::string Summary;
	};

	/// Calculates and logs the state hash after executing nemesis \a blockElement for network configured with \a config
	/// with specified cache database cleanup mode (\a databaseCleanupMode).
	NemesisStateHashDescriptor CalculateAndLogNemesisStateHash(
			const model::BlockElement& blockElement,
			const config::LocalNodeConfiguration& config,
			CacheDatabaseCleanupMode databaseCleanupMode);
}}}

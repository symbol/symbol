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
#include "catapult/types.h"
#include <string>

namespace catapult {
	namespace config { class CatapultConfiguration; }
	namespace model {
		struct BlockElement;
		struct BlockStatement;
	}
}

namespace catapult { namespace tools { namespace nemgen {

	/// Possible cache database cleanup modes.
	enum class CacheDatabaseCleanupMode {
		/// Perform no cleanup.
		None,

		/// Purge after execution.
		Purge
	};

	/// Nemesis block execution dependent hashes information.
	struct NemesisExecutionHashesDescriptor {
		/// Receipts hash.
		Hash256 ReceiptsHash;

		/// State hash.
		Hash256 StateHash;

		/// Textual summary including sub cache hashes.
		std::string Summary;

		/// Block statement.
		std::unique_ptr<model::BlockStatement> pBlockStatement;
	};

	/// Calculates and logs the nemesis block execution dependent hashes after executing nemesis \a blockElement
	/// for network configured with \a config with specified cache database cleanup mode (\a databaseCleanupMode).
	NemesisExecutionHashesDescriptor CalculateAndLogNemesisExecutionHashes(
			const model::BlockElement& blockElement,
			const config::CatapultConfiguration& config,
			CacheDatabaseCleanupMode databaseCleanupMode);
}}}

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
#include "catapult/model/Block.h"
#include "catapult/model/TransactionSelectionStrategy.h"

namespace catapult {
	namespace cache { class ReadWriteUtCache; }
	namespace harvesting { class HarvestingUtFacadeFactory; }
}

namespace catapult { namespace harvesting {

	/// Generates a block from a seed block header given a maximum number of transactions.
	using BlockGenerator = std::function<std::unique_ptr<model::Block> (const model::BlockHeader&, uint32_t)>;

	/// Creates a default block generator around \a utFacadeFactory and \a utCache for specified transaction \a strategy.
	BlockGenerator CreateHarvesterBlockGenerator(
			model::TransactionSelectionStrategy strategy,
			const HarvestingUtFacadeFactory& utFacadeFactory,
			const cache::ReadWriteUtCache& utCache);
}}

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
#include "MapperInclude.h"

namespace catapult {
	namespace model {
		struct BlockElement;
		struct FinalizationRound;
	}
}

namespace catapult { namespace mongo { namespace mappers {

	/// Maps \a blockElement to the corresponding db model value given \a totalTransactionsCount.
	bsoncxx::document::value ToDbModel(const model::BlockElement& blockElement, uint32_t totalTransactionsCount);

	/// Maps a finalized block with \a height and \a hash at finalization \a round to to the corresponding db model value.
	bsoncxx::document::value ToDbModel(const model::FinalizationRound& round, Height height, const Hash256& hash);
}}}

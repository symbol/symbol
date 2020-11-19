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

#include "MoveBlockFiles.h"
#include "BlockStatementSerializer.h"
#include "BlockStorage.h"
#include "BufferInputStreamAdapter.h"

namespace catapult { namespace io {

	void MoveBlockFiles(PrunableBlockStorage& sourceStorage, BlockStorage& destinationStorage, Height startHeight) {
		if (startHeight < Height(1))
			CATAPULT_THROW_INVALID_ARGUMENT_1("invalid height passed", startHeight);

		if (startHeight <= destinationStorage.chainHeight())
			destinationStorage.dropBlocksAfter(startHeight - Height(1));

		auto sourceHeight = sourceStorage.chainHeight();
		for (auto height = startHeight; height <= sourceHeight; height = height + Height(1)) {
			auto pBlockElement = sourceStorage.loadBlockElement(height);
			auto blockStatementPair = sourceStorage.loadBlockStatementData(height);

			if (blockStatementPair.second) {
				auto pBlockStatement = std::make_shared<model::BlockStatement>();
				BufferInputStreamAdapter<std::vector<uint8_t>> blockStatementStream(blockStatementPair.first);
				ReadBlockStatement(blockStatementStream, *pBlockStatement);
				const_cast<model::BlockElement&>(*pBlockElement).OptionalStatement = std::move(pBlockStatement);
			}

			destinationStorage.saveBlock(*pBlockElement);
		}

		sourceStorage.purge();
	}
}}

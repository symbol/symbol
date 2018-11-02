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

#include "LocalNodeStateHashTestUtils.h"
#include "catapult/io/RawFile.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/nodeps/MijinConstants.h"

namespace catapult { namespace test {

	void SetNemesisStateHash(const std::string& destination, const config::LocalNodeConfiguration& config) {
		// calculate the state hash (default nemesis block has zeroed state hash)
		mocks::MockMemoryBlockStorage storage;
		auto pNemesisBlockElement = storage.loadBlockElement(Height(1));
		auto nemesisStateHash = CalculateNemesisStateHash(*pNemesisBlockElement, config);

		// add state hash to nemesis block and resign it
		auto& nemesisBlock = const_cast<model::Block&>(pNemesisBlockElement->Block);
		nemesisBlock.StateHash = nemesisStateHash;
		SignBlock(crypto::KeyPair::FromString(Mijin_Test_Nemesis_Private_Key), nemesisBlock);

		// overwrite the nemesis file in destination
		// (only the block and entity hash need to be rewritten; this works because block size does not change)
		io::RawFile nemesisFile(destination + "/00000/00001.dat", io::OpenMode::Read_Append);
		nemesisFile.write({ reinterpret_cast<const uint8_t*>(&nemesisBlock), nemesisBlock.Size });
		nemesisFile.write(model::CalculateHash(nemesisBlock));
	}
}}

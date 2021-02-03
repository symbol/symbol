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
#include "catapult/functions.h"
#include <string>

namespace catapult {
	namespace model {
		struct Block;
		struct BlockElement;
	}
}

namespace catapult { namespace test {

	/// Prepares the storage by creating the \a destination directory structure and seeding a nemesis block.
	void PrepareStorage(const std::string& destination);

	/// Prepares the seed storage by creating the \a destination directory structure and seeding a nemesis block.
	void PrepareSeedStorage(const std::string& destination);

	/// Prepares the storage by creating the \a destination directory structure.
	void PrepareStorageWithoutNemesis(const std::string& destination);

	/// Modifies the nemesis block stored in \a destination by applying \a modify.
	void ModifyNemesis(const std::string& destination, const consumer<model::Block&, const model::BlockElement&>& modify);

	/// Modifies the seed nemesis block stored in \a destination by applying \a modify.
	void ModifySeedNemesis(const std::string& destination, const consumer<model::Block&, const model::BlockElement&>& modify);

	/// Fakes file-based chain located at \a destination to \a height
	/// by setting proper value in index.dat and filling 00000/hashes.dat.
	void FakeHeight(const std::string& destination, uint64_t height);
}}

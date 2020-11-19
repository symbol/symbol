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

#include "PatriciaTreeTestUtils.h"

namespace catapult { namespace test {

	namespace {
		template<typename TTree>
		void SeedTreeWithFourNodesT(TTree& tree) {
			tree.set(0x64'6F'00'00, "verb");
			tree.set(0x64'6F'67'00, "puppy");
			tree.set(0x64'6F'67'65, "coin");
			tree.set(0x68'6F'72'73, "stallion");
		}
	}

	void SeedTreeWithFourNodes(MemoryPatriciaTree& tree) {
		SeedTreeWithFourNodesT(tree);
	}

	void SeedTreeWithFourNodes(MemoryBasePatriciaTree& tree) {
		auto pDeltaTree = tree.rebase();
		SeedTreeWithFourNodesT(*pDeltaTree);
		tree.commit();
	}

	Hash256 CalculateRootHash(const std::vector<std::pair<uint32_t, std::string>>& pairs) {
		tree::MemoryDataSource dataSource;
		MemoryPatriciaTree tree(dataSource);

		for (const auto& pair : pairs)
			tree.set(pair.first, pair.second);

		return tree.root();
	}
}}

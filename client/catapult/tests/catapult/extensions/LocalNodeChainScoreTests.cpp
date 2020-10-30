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

#include "catapult/extensions/LocalNodeChainScore.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS LocalNodeChainScoreTests

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultChainScore) {
		// Act:
		LocalNodeChainScore score;

		// Assert:
		auto scoreArray = score.get().toArray();
		EXPECT_EQ(0u, scoreArray[0]);
		EXPECT_EQ(0u, scoreArray[1]);
	}

	TEST(TEST_CLASS, CanCreateChainScoreFromModelChainScore) {
		// Act:
		LocalNodeChainScore score(model::ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6));

		// Assert:
		auto scoreArray = score.get().toArray();
		EXPECT_EQ(0x8FDE'4267'9C23'D678u, scoreArray[0]);
		EXPECT_EQ(0x7A6B'3481'0235'43B6u, scoreArray[1]);
	}

	// endregion

	// region set

	TEST(TEST_CLASS, CanSetNewChainScore) {
		// Act:
		LocalNodeChainScore score(model::ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6));
		score.set(model::ChainScore(0x9FDE'426A'8C23'D688u, 0x7C6B'3981'0265'43BBu));

		// Assert:
		auto scoreArray = score.get().toArray();
		EXPECT_EQ(0x9FDE'426A'8C23'D688u, scoreArray[0]);
		EXPECT_EQ(0x7C6B'3981'0265'43BBu, scoreArray[1]);
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddToChainScore) {
		// Arrange:
		LocalNodeChainScore score(model::ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6));

		// Act:
		const auto& result = score += model::ChainScore(0x1000'0002'F000'0010, 0x0200'0500'0030'0005);

		// Assert:
		auto scoreArray = score.get().toArray();
		EXPECT_EQ(&score, &result);
		EXPECT_EQ(0x9FDE'426A'8C23'D688u, scoreArray[0]);
		EXPECT_EQ(0x7C6B'3981'0265'43BBu, scoreArray[1]);
	}

	TEST(TEST_CLASS, CanAddToChainScore_Delta) {
		// Arrange:
		LocalNodeChainScore score(model::ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6));

		// Act:
		const auto& result = score += model::ChainScore::Delta(0x0200'0500'0030'0005);

		// Assert:
		auto scoreArray = score.get().toArray();
		EXPECT_EQ(&score, &result);
		EXPECT_EQ(0x8FDE'4267'9C23'D678u, scoreArray[0]);
		EXPECT_EQ(0x7C6B'3981'0265'43BBu, scoreArray[1]);
	}

	// endregion
}}

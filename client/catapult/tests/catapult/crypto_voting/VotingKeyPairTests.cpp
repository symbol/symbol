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

#include "catapult/crypto_voting/VotingKeyPair.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS VotingKeyPairTests

	TEST(TEST_CLASS, KeyPairPassesNemTestVectors) {
		// Arrange: from nem https://github.com/nemtech/test-vectors
		std::string dataSet[] {
			"ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57",
			"FE9BC2EF8DF88E708CAB471F82B54DBFCBA11B121E7C2D02799AB4D3A53F0E5B",
			"DAEE5A32E12CEDEFD0349FDBA1FCBDB45356CA3A35AA5CF1A8AE1091BBA98B73",
			"A6954BAA315EE50453CCE3483906F134405B8B3ADD94BFC8D8125CF3C09BBFE8",
			"832A237053A83B7E97CA287AC15F9AD5838898E7395967B56D39749652EA25C3"
		};
		std::string expectedSet[] {
			"5112BA143B78132AF616AF1A94E911EAD890FDB51B164A1B57C352ECD9CA1894",
			"5F9EB725880D0B8AC122AD2939070172C8762713A1E29CE55EEEA0BFBA05E6DB",
			"2D8C6B2B1D69CC02464339F46A788D7A5A6D7875C9D12AAD4ACCF2D5B24887FC",
			"20E7F2BC716306F70A136121DC103604FD624328BCEA81E5786F3CB4CE96E60E",
			"2470623117B439AA09C487D0F3D4B23676565DC1478F7C7443579B0255FE6DE1"
		};

		ASSERT_EQ(CountOf(dataSet), CountOf(expectedSet));
		for (size_t i = 0; i < CountOf(dataSet); ++i) {
			// Act:
			auto keyPair = VotingKeyPair::FromString(dataSet[i]);

			// Assert:
			EXPECT_EQ(utils::ParseByteArray<VotingKey>(expectedSet[i]), keyPair.publicKey());
		}
	}
}}

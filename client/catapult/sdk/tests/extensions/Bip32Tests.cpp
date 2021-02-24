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

#include "src/extensions/Bip32.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS Bip32Tests

	namespace {
		constexpr const auto Deterministic_Seed = "000102030405060708090A0B0C0D0E0F";

		void AssertBip32Node(
				Bip32Node&& node,
				const Hash256& expectedChainCode,
				const crypto::PrivateKey& expectedPrivateKey,
				const std::string& message = std::string()) {
			EXPECT_EQ(expectedChainCode, node.chainCode()) << message;

			auto keyPair = Bip32Node::ExtractKeyPair(std::move(node));
			EXPECT_EQ(expectedPrivateKey, keyPair.privateKey()) << message;
		}
	}

	TEST(TEST_CLASS, CanCreateRootNode) {
		// Arrange:
		auto seed = test::HexStringToVector(Deterministic_Seed);

		// Act:
		auto node = Bip32Node::FromSeed(seed);

		// Assert:
		AssertBip32Node(
				std::move(node),
				utils::ParseByteArray<Hash256>("90046A93DE5380A72B5E45010748567D5EA02BBF6522F979E05C0D8D8CA9FFFB"),
				crypto::PrivateKey::FromString("2B4BE7F19EE27BBF30C667B642D5F4AA69FD169872F8FC3059C08EBAE2EB19E7"));
	}

	TEST(TEST_CLASS, CanDeriveSingleLevelNode) {
		// Arrange:
		auto seed = test::HexStringToVector(Deterministic_Seed);

		// Act:
		auto node = Bip32Node::FromSeed(seed).derive(0);

		// Assert:
		AssertBip32Node(
				std::move(node),
				utils::ParseByteArray<Hash256>("8B59AA11380B624E81507A27FEDDA59FEA6D0B779A778918A2FD3590E16E9C69"),
				crypto::PrivateKey::FromString("68E0FE46DFB67E368C75379ACEC591DAD19DF3CDE26E63B93A8E704F1DADE7A3"));
	}

	namespace {
		void AssertWellKnownChildrenFromDeterministicSeed(Bip32Node&& childNode0, Bip32Node&& childNode1) {
			AssertBip32Node(
					std::move(childNode0),
					utils::ParseByteArray<Hash256>("B8E16D407C8837B46A9445C6417310F3C7A4DCD9B8FF2679C383E6DEF721AC11"),
					crypto::PrivateKey::FromString("BB2724A538CFD64E4366FEB36BB982B954D58EA78F7163451B3B514EDD692159"),
					"child node 0");

			AssertBip32Node(
					std::move(childNode1),
					utils::ParseByteArray<Hash256>("68CA2A058611AAC20CAFB4E1CCD70961E67D8C567390B3CBFC63D0E58BAE7153"),
					crypto::PrivateKey::FromString("8C91D9F5D214A2E80A275E75A165F7022712F7AD52B7ECD45B3B6CC76154B571"),
					"child node 1");
		}
	}

	TEST(TEST_CLASS, CanDeriveChildNodes_Chained) {
		// Arrange:
		auto seed = test::HexStringToVector(Deterministic_Seed);

		// Act:
		auto node = Bip32Node::FromSeed(seed);
		auto childNode0 = node.derive(44).derive(4343).derive(0).derive(0).derive(0);
		auto childNode1 = node.derive(44).derive(4343).derive(1).derive(0).derive(0);

		// Assert:
		AssertWellKnownChildrenFromDeterministicSeed(std::move(childNode0), std::move(childNode1));
	}

	TEST(TEST_CLASS, CanDeriveChildNodes_Path) {
		// Arrange:
		auto seed = test::HexStringToVector(Deterministic_Seed);

		// Act:
		auto node = Bip32Node::FromSeed(seed);
		auto childNode0 = node.derive({ 44, 4343, 0, 0, 0, });
		auto childNode1 = node.derive({ 44, 4343, 1, 0, 0, });

		// Assert:
		AssertWellKnownChildrenFromDeterministicSeed(std::move(childNode0), std::move(childNode1));
	}
}}

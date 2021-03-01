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

	// region basic tests

	namespace {
		constexpr const auto Deterministic_Seed = "000102030405060708090A0B0C0D0E0F";

		void AssertBip32Node(
				Bip32Node&& node,
				const Hash256& expectedChainCode,
				crypto::PrivateKey&& expectedPrivateKey,
				const std::string& message = std::string()) {
			EXPECT_EQ(expectedChainCode, node.chainCode()) << message;

			auto expectedKeyPair = crypto::KeyPair::FromPrivate(std::move(expectedPrivateKey));
			EXPECT_EQ(expectedKeyPair.publicKey(), node.publicKey()) << message;

			auto keyPair = Bip32Node::ExtractKeyPair(std::move(node));
			EXPECT_EQ(expectedKeyPair.privateKey(), keyPair.privateKey()) << message;
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

	// endregion

	// region test vectors

	namespace {
		// data from: https://github.com/satoshilabs/slips/blob/master/slip-0010.md#test-vector-1-for-ed25519
		struct TestVectorBip32 {
			std::string Seed;
			std::vector<uint32_t> Path;
			std::string ChainCode;
			std::string PublicKey;
		};

		constexpr auto Test_Vector_Seed_1 = "000102030405060708090A0B0C0D0E0F";
		constexpr auto Test_Vector_Seed_2 =
				"FFFCF9F6F3F0EDEAE7E4E1DEDBD8D5D2CFCCC9C6C3C0BDBAB7B4B1AEABA8A5A2"
				"9F9C999693908D8A8784817E7B7875726F6C696663605D5A5754514E4B484542";

		std::vector<TestVectorBip32> GetTestVectorsBip32() {
			return {
				{
					Test_Vector_Seed_1,
					{},
					"90046A93DE5380A72B5E45010748567D5EA02BBF6522F979E05C0D8D8CA9FFFB",
					"A4B2856BFEC510ABAB89753FAC1AC0E1112364E7D250545963F135F2A33188ED"
				},
				{
					Test_Vector_Seed_1,
					{ 0 },
					"8B59AA11380B624E81507A27FEDDA59FEA6D0B779A778918A2FD3590E16E9C69",
					"8C8A13DF77A28F3445213A0F432FDE644ACAA215FC72DCDF300D5EFAA85D350C"
				},
				{
					Test_Vector_Seed_1,
					{ 0, 1 },
					"A320425F77D1B5C2505A6B1B27382B37368EE640E3557C315416801243552F14",
					"1932A5270F335BED617D5B935C80AEDB1A35BD9FC1E31ACAFD5372C30F5C1187"
				},
				{
					Test_Vector_Seed_1,
					{ 0, 1, 2 },
					"2E69929E00B5AB250F49C3FB1C12F252DE4FED2C1DB88387094A0F8C4C9CCD6C",
					"AE98736566D30ED0E9D2F4486A64BC95740D89C7DB33F52121F8EA8F76FF0FC1"
				},
				{
					Test_Vector_Seed_1,
					{ 0, 1, 2, 2 },
					"8F6D87F93D750E0EFCCDA017D662A1B31A266E4A6F5993B15F5C1F07F74DD5CC",
					"8ABAE2D66361C879B900D204AD2CC4984FA2AA344DD7DDC46007329AC76C429C"
				},
				{
					Test_Vector_Seed_1,
					{ 0, 1, 2, 2, 1000000000 },
					"68789923A0CAC2CD5A29172A475FE9E0FB14CD6ADB5AD98A3FA70333E7AFA230",
					"3C24DA049451555D51A7014A37337AA4E12D41E485ABCCFA46B47DFB2AF54B7A"
				},
				{
					Test_Vector_Seed_2,
					{},
					"EF70A74DB9C3A5AF931B5FE73ED8E1A53464133654FD55E7A66F8570B8E33C3B",
					"8FE9693F8FA62A4305A140B9764C5EE01E455963744FE18204B4FB948249308A"
				},
				{
					Test_Vector_Seed_2,
					{ 0 },
					"0B78A3226F915C082BF118F83618A618AB6DEC793752624CBEB622ACB562862D",
					"86FAB68DCB57AA196C77C5F264F215A112C22A912C10D123B0D03C3C28EF1037"
				},
				{
					Test_Vector_Seed_2,
					{ 0, 2147483647 },
					"138F0B2551BCAFECA6FF2AA88BA8ED0ED8DE070841F0C4EF0165DF8181EAAD7F",
					"5BA3B9AC6E90E83EFFCD25AC4E58A1365A9E35A3D3AE5EB07B9E4D90BCF7506D"
				},
				{
					Test_Vector_Seed_2,
					{ 0, 2147483647, 1 },
					"73BD9FFF1CFBDE33A1B846C27085F711C0FE2D66FD32E139D3EBC28E5A4A6B90",
					"2E66AA57069C86CC18249AECF5CB5A9CEBBFD6FADEAB056254763874A9352B45"
				},
				{
					Test_Vector_Seed_2,
					{ 0, 2147483647, 1, 2147483646 },
					"0902FE8A29F9140480A00EF244BD183E8A13288E4412D8389D140AAC1794825A",
					"E33C0F7D81D843C572275F287498E8D408654FDF0D1E065B84E2E6F157AAB09B"
				},
				{
					Test_Vector_Seed_2,
					{ 0, 2147483647, 1, 2147483646, 2 },
					"5D70AF781F3A37B829F0D060924D5E960BDC02E85423494AFC0B1A41BBE196D4",
					"47150C75DB263559A70D5778BF36ABBAB30FB061AD69F69ECE61A72B0CFA4FC0"
				}
			};
		}
	}

	TEST(TEST_CLASS, Bip32TestVectors) {
		// Arrange:
		auto i = 0u;
		for (const auto& testVector : GetTestVectorsBip32()) {
			auto seed = test::HexStringToVector(testVector.Seed);

			// Act:
			auto node = Bip32Node::FromSeed(seed);
			for (auto id : testVector.Path)
				node = node.derive(id);

			// Assert:
			EXPECT_EQ(utils::ParseByteArray<Hash256>(testVector.ChainCode), node.chainCode()) << "test vector at " << i;
			EXPECT_EQ(utils::ParseByteArray<Key>(testVector.PublicKey), node.publicKey()) << "test vector at " << i;
			++i;
		}
	}

	// endregion
}}

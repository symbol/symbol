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

#include "catapult/tree/PatriciaTreeSerializer.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace tree {

#define TEST_CLASS PatriciaTreeSerializerTests

	using Serializer = PatriciaTreeSerializer;

	// region (PTSERIALIZER) traits

	namespace {
		template<size_t Size, size_t NibbleSize>
		struct BasePathTraits {
			static constexpr auto Path_Size = Size;
			static constexpr auto Path_Nibble_Size = NibbleSize;
		};

		struct OddPathTraits : public BasePathTraits<6, 11> {
			static auto CreatePath() {
				auto path = TreeNodePath(test::GenerateRandomArray<Path_Size>());
				return path.subpath(1);
			}
		};

		struct EvenPathTraits : public BasePathTraits<7, 14> {
			static auto CreatePath() {
				return TreeNodePath(test::GenerateRandomArray<Path_Size>());
			}
		};

		struct EmptyPathTraits : public BasePathTraits<0, 0> {
			static auto CreatePath() {
				return TreeNodePath();
			}
		};

		template<typename TPathTraits>
		auto AssertCommonData(uint8_t marker, const TreeNodePath& path, const std::string& result) {
			EXPECT_EQ(marker, static_cast<uint8_t>(result[0]));

			auto pathSize = *reinterpret_cast<const uint8_t*>(result.data() + sizeof(uint8_t));
			EXPECT_EQ(TPathTraits::Path_Nibble_Size, pathSize);

			const auto* pPath = reinterpret_cast<const uint8_t*>(result.data() + 2 * sizeof(uint8_t));
			std::vector<uint8_t> rawPath(pPath, pPath + TPathTraits::Path_Size);
			TreeNodePath resultPath(rawPath);
			EXPECT_EQ(path, resultPath.subpath(0, TPathTraits::Path_Nibble_Size));

			return pPath + TPathTraits::Path_Size;
		}

		template<typename TPathTraits>
		void AssertSerializedLeaf(const LeafTreeNode& value, const std::string& result) {
			auto expectedSize = 2 * sizeof(uint8_t) + TPathTraits::Path_Size + Hash256::Size;
			ASSERT_EQ(expectedSize, result.size());

			const auto* pData = AssertCommonData<TPathTraits>(0xFF, value.path(), result);

			const auto& hash = reinterpret_cast<const Hash256&>(*pData);
			EXPECT_EQ(value.value(), hash);
		}

		template<typename TPathTraits, typename TIndexGenerator>
		void AssertSerializedBranch(
				const BranchTreeNode& value,
				size_t numLinks,
				TIndexGenerator indexGenerator,
				uint16_t expectedLinksMask,
				const std::string& result) {
			auto expectedSize = 2 * sizeof(uint8_t) + TPathTraits::Path_Size + sizeof(uint16_t) + numLinks * Hash256::Size;
			ASSERT_EQ(expectedSize, result.size());

			const auto* pData = AssertCommonData<TPathTraits>(0, value.path(), result);

			auto linksMask = reinterpret_cast<const uint16_t&>(*pData);
			EXPECT_EQ(expectedLinksMask, linksMask);

			const auto* pHashes = reinterpret_cast<const Hash256*>(pData + sizeof(uint16_t));
			for (auto i = 0u; i < numLinks; ++i)
				EXPECT_EQ(value.link(indexGenerator(i)), pHashes[i]) << "link index: " << i;
		}
	}

#define PTSERIALIZER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TPathTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_OddPath) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<OddPathTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_EvenPath) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EvenPathTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_EmptyPath) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmptyPathTraits>(); } \
	template<typename TPathTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region Serialize - value

	TEST(TEST_CLASS, SerializeNodeFailsWhenNodeIsEmpty) {
		EXPECT_THROW(Serializer::SerializeValue(TreeNode()), catapult_invalid_argument);
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanSerializeLeaf) {
		// Arrange:
		auto leafValue = test::GenerateRandomByteArray<Hash256>();
		auto originalNode = LeafTreeNode(TPathTraits::CreatePath(), leafValue);

		// Act:
		auto result = Serializer::SerializeValue(TreeNode(originalNode));

		// Assert:
		AssertSerializedLeaf<TPathTraits>(originalNode, result);
	}

	namespace {
		template<typename TPathTraits, typename TIndexGenerator>
		void AssertCanSerializeBranch(size_t numLinks, TIndexGenerator indexGenerator, uint16_t expectedMask) {
			// Arrange:
			auto originalNode = BranchTreeNode(TPathTraits::CreatePath());
			for (auto i = 0u; i < numLinks; ++i)
				originalNode.setLink(test::GenerateRandomByteArray<Hash256>(), indexGenerator(i));

			// Act:
			auto result = Serializer::SerializeValue(TreeNode(originalNode));

			// Assert:
			AssertSerializedBranch<TPathTraits>(originalNode, numLinks, indexGenerator, expectedMask, result);
		}
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanSerializeBranchWithoutLinks) {
		AssertCanSerializeBranch<TPathTraits>(0, [](auto) { return 0u; }, 0x0000);
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanSerializeBranchWithLinks) {
		// Act + Assert: mask: 0010'0100'1001'0010 = 0x2492
		AssertCanSerializeBranch<TPathTraits>(5, [](auto i) { return i * 3 + 1u; }, 0x2492);
	}

	// endregion

	// region Deserialize - helpers

	namespace {
		template<typename TDestContainer, typename TSourceContainer>
		void Append(TDestContainer& dest, const TSourceContainer& source) {
			dest.insert(dest.end(), source.cbegin(), source.cend());
		}

		auto CreateNodeHeader(uint8_t marker, uint8_t numNibbles, const std::vector<uint8_t>& rawPath) {
			std::vector<uint8_t> buffer;
			buffer.push_back(marker);
			buffer.push_back(numNibbles);
			Append(buffer, rawPath);
			return buffer;
		}

		auto CreateSerializedLeaf(const std::vector<uint8_t>& rawPath, size_t size, const Hash256& value) {
			auto buffer = CreateNodeHeader(0xFF, static_cast<uint8_t>(size), rawPath);
			Append(buffer, value);
			return buffer;
		}

		auto CreateSerializedBranch(
				const std::vector<uint8_t>& rawPath,
				size_t size,
				uint16_t linksMask,
				const std::vector<Hash256>& links) {
			auto buffer = CreateNodeHeader(0x00, static_cast<uint8_t>(size), rawPath);

			std::vector<uint8_t> serializedLinksMask(sizeof(uint16_t));
			reinterpret_cast<uint16_t&>(serializedLinksMask[0]) = linksMask;
			Append(buffer, serializedLinksMask);

			for (const auto& link : links)
				Append(buffer, link);

			return buffer;
		}

		template<typename TPathTraits>
		auto GenerateValidPath() {
			auto rawPath = test::GenerateRandomDataVector<uint8_t>(TPathTraits::Path_Size);
			bool isOdd = 1 == TPathTraits::Path_Nibble_Size % 2;
			if (isOdd)
				rawPath[TPathTraits::Path_Size - 1] &= 0xF0;

			return rawPath;
		}

		template<typename TPathTraits>
		auto GenerateInvalidPath() {
			auto rawPath = GenerateValidPath<TPathTraits>();
			rawPath[TPathTraits::Path_Size - 1] |= 0x0F;
			return rawPath;
		}
	}

	// endregion

	// region Deserialize - not enough data

	PTSERIALIZER_TRAITS_BASED_TEST(DeserializeFailsWhenThereIsNotEnoughData_Leaf) {
		// Arrange:
		auto rawPath = GenerateValidPath<TPathTraits>();
		auto value = test::GenerateRandomByteArray<Hash256>();
		auto buffer = CreateSerializedLeaf(rawPath, TPathTraits::Path_Nibble_Size, value);

		// - drop last byte
		buffer.resize(buffer.size() - 1);

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(buffer), catapult_file_io_error);
	}

	PTSERIALIZER_TRAITS_BASED_TEST(DeserializeFailsWhenThereIsNotEnoughData_Branch) {
		// Arrange:
		auto rawPath = GenerateValidPath<TPathTraits>();
		auto buffer = CreateSerializedBranch(rawPath, TPathTraits::Path_Nibble_Size, 0x0000, {});

		// - drop last byte
		buffer.resize(buffer.size() - 1);

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(buffer), catapult_file_io_error);
	}

	// endregion

	// region Deserialize - invalid marker

	PTSERIALIZER_TRAITS_BASED_TEST(DeserializeFailsWhenMarkerIsInvalid_Leaf) {
		// Arrange:
		auto rawPath = GenerateValidPath<TPathTraits>();
		auto value = test::GenerateRandomByteArray<Hash256>();
		auto buffer = CreateSerializedLeaf(rawPath, TPathTraits::Path_Nibble_Size, value);

		// - change marker
		buffer[0] = 0x34;

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(buffer), catapult_invalid_argument);
	}

	// endregion

	// region Deserialize - path deserialization throws

	TEST(TEST_CLASS, DeserializeValueFailsWhenPathSizeIsOddAndHasWrongFillerNibble) {
		// Arrange: make sure filler nibble is non-zero
		auto rawPath = GenerateInvalidPath<OddPathTraits>();
		auto value = test::GenerateRandomByteArray<Hash256>();
		auto buffer = CreateSerializedLeaf(rawPath, OddPathTraits::Path_Nibble_Size, value);

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(buffer), catapult_runtime_error);
	}

	TEST(TEST_CLASS, DeserializeValueDoesNotFailWhenPathSizeIsEven) {
		// Arrange:
		auto rawPath = GenerateInvalidPath<EvenPathTraits>();
		auto value = test::GenerateRandomByteArray<Hash256>();
		auto buffer = CreateSerializedLeaf(rawPath, EvenPathTraits::Path_Nibble_Size, value);

		// Act + Assert:
		EXPECT_NO_THROW(Serializer::DeserializeValue(buffer));
	}

	// endregion

	// region Deserialize - valid

	PTSERIALIZER_TRAITS_BASED_TEST(CanDeserializeLeaf) {
		// Arrange:
		auto rawPath = GenerateValidPath<TPathTraits>();
		auto value = test::GenerateRandomByteArray<Hash256>();
		auto buffer = CreateSerializedLeaf(rawPath, TPathTraits::Path_Nibble_Size, value);
		auto path = TreeNodePath(rawPath);

		// Act:
		auto node = Serializer::DeserializeValue(buffer);

		// Assert:
		EXPECT_TRUE(node.isLeaf());
		const auto& leafNode = node.asLeafNode();
		EXPECT_EQ(TPathTraits::Path_Nibble_Size, leafNode.path().size());
		EXPECT_EQ(TPathTraits::Path_Nibble_Size, FindFirstDifferenceIndex(path, leafNode.path()));
		EXPECT_EQ(value, leafNode.value());
	}

	namespace {
		void AssertBranchLinks(const std::vector<Hash256>& links, uint16_t linksMask, const BranchTreeNode& branchNode) {
			auto linkIndex = 0u;
			for (auto i = 0u; i < 16; ++i) {
				if (0 == (linksMask & (1 << i))) {
					EXPECT_FALSE(branchNode.hasLink(i)) << "unexpected link at: " << i;
				} else {
					EXPECT_TRUE(branchNode.hasLink(i)) << "expected link at: " << i;
					EXPECT_EQ(links[linkIndex], branchNode.link(i)) << " invalid hash: " << linkIndex << " link index: " << i;
					++linkIndex;
				}
			}
		}

		template<typename TPathTraits>
		void AssertCanDeserializeBranch(size_t numLinks, uint16_t linksMask) {
			// Arrange:
			auto rawPath = GenerateValidPath<TPathTraits>();
			auto links = test::GenerateRandomDataVector<Hash256>(numLinks);
			auto buffer = CreateSerializedBranch(rawPath, TPathTraits::Path_Nibble_Size, linksMask, links);
			auto path = TreeNodePath(rawPath);

			// Act:
			auto node = Serializer::DeserializeValue(buffer);

			// Assert:
			EXPECT_TRUE(node.isBranch());
			const auto& branchNode = node.asBranchNode();
			EXPECT_EQ(TPathTraits::Path_Nibble_Size, branchNode.path().size());
			EXPECT_EQ(TPathTraits::Path_Nibble_Size, FindFirstDifferenceIndex(path, branchNode.path()));
			EXPECT_EQ(numLinks, branchNode.numLinks());
			AssertBranchLinks(links, linksMask, branchNode);
		}
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanDeserializeBranchWithoutLinks) {
		AssertCanDeserializeBranch<TPathTraits>(0, 0x0000);
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanDeserializeBranchWithLinks) {
		AssertCanDeserializeBranch<TPathTraits>(4, 0x8421);
	}

	// endregion

	// region Roundtrip

	PTSERIALIZER_TRAITS_BASED_TEST(CanRoundtripLeaf) {
		// Arrange:
		auto leafValue = test::GenerateRandomByteArray<Hash256>();
		auto originalNode = LeafTreeNode(TPathTraits::CreatePath(), leafValue);

		// Act:
		auto result = test::RunRoundtripStringTest<Serializer>(TreeNode(originalNode));

		// Assert:
		EXPECT_EQ(originalNode.path(), result.path());
		EXPECT_EQ(originalNode.hash(), result.hash());
	}

	namespace {
		template<typename TPathTraits, typename TIndexGenerator>
		void AssertCanRoundtripBranch(size_t numLinks, TIndexGenerator indexGenerator) {
			// Arrange:
			auto originalNode = BranchTreeNode(TPathTraits::CreatePath());
			for (auto i = 0u; i < numLinks; ++i)
				originalNode.setLink(test::GenerateRandomByteArray<Hash256>(), indexGenerator(i));

			// Act:
			auto result = test::RunRoundtripStringTest<Serializer>(TreeNode(originalNode));

			// Assert:
			EXPECT_EQ(originalNode.path(), result.path());
			EXPECT_EQ(originalNode.hash(), result.hash());
			EXPECT_EQ(numLinks, result.asBranchNode().numLinks());
		}
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanRoundtripBranchWithoutLinks) {
		AssertCanRoundtripBranch<TPathTraits>(0, [](auto) { return 0u; });
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanRoundtripBranchWithLinks) {
		// Act + Assert: mask: 0010'0100'1001'0010 = 0x2492
		AssertCanRoundtripBranch<TPathTraits>(5, [](auto i) { return i * 3 + 1u; });
	}

	// endregion
}}

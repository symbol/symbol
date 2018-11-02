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

#include "catapult/tree/PatriciaTreeSerializer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace tree {

#define TEST_CLASS PatriciaTreeSerializerTests

	using Serializer = PatriciaTreeSerializer;

	// region serialization - value

	TEST(TEST_CLASS, SerializeNodeFailsIfNodeIsEmpty) {
		// Act + Assert:
		EXPECT_THROW(Serializer::SerializeValue(TreeNode()), catapult_invalid_argument);
	}

	namespace {
		struct OddPathTraits {
		public:
			constexpr static size_t PathSize() {
				return 6;
			}

			constexpr static size_t PathNibbleSize() {
				return 11;
			}

		public:
			static auto CreatePath() {
				auto path = TreeNodePath(test::GenerateRandomData<PathSize()>());
				return path.subpath(1);
			}
		};

		struct EvenPathTraits {
		public:
			constexpr static size_t PathSize() {
				return 7;
			}

			constexpr static size_t PathNibbleSize() {
				return 14;
			}

		public:
			static auto CreatePath() {
				return TreeNodePath(test::GenerateRandomData<PathSize()>());
			}
		};

		template<typename TPathTraits>
		auto AssertCommonData(uint8_t marker, const TreeNodePath& path, const std::string& result) {
			EXPECT_EQ(marker, static_cast<uint8_t>(result[0]));

			auto pathSize = *reinterpret_cast<const uint8_t*>(result.data() + sizeof(uint8_t));
			EXPECT_EQ(TPathTraits::PathNibbleSize(), pathSize);

			const auto* pPath = reinterpret_cast<const uint8_t*>(result.data() + 2 * sizeof(uint8_t));
			std::vector<uint8_t> rawPath(pPath, pPath + TPathTraits::PathSize());
			TreeNodePath resultPath(rawPath);
			EXPECT_EQ(path, resultPath.subpath(0, TPathTraits::PathNibbleSize()));

			return pPath + TPathTraits::PathSize();
		}

		template<typename TPathTraits>
		void AssertSerializedLeaf(const LeafTreeNode& value, const std::string& result) {
			auto expectedSize = 2 * sizeof(uint8_t) + TPathTraits::PathSize() + Hash256_Size;
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
			auto expectedSize = 2 * sizeof(uint8_t) + TPathTraits::PathSize() + sizeof(uint16_t) + numLinks * Hash256_Size;
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
	template<typename TPathTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	PTSERIALIZER_TRAITS_BASED_TEST(CanSerializeLeaf) {
		// Arrange:
		auto leafValue = test::GenerateRandomData<Hash256_Size>();
		auto node = LeafTreeNode(TPathTraits::CreatePath(), leafValue);

		// Act:
		auto result = Serializer::SerializeValue(TreeNode(node));

		// Assert:
		AssertSerializedLeaf<TPathTraits>(node, result);
	}

	namespace {
		template<typename TPathTraits, typename TIndexGenerator>
		void AssertCanSerializeBranch(size_t numLinks, TIndexGenerator indexGenerator, uint16_t expectedMask) {
			// Arrange:
			auto node = BranchTreeNode(TPathTraits::CreatePath());
			for (auto i = 0u; i < numLinks; ++i)
				node.setLink(test::GenerateRandomData<Hash256_Size>(), indexGenerator(i));

			// Act:
			auto result = Serializer::SerializeValue(TreeNode(node));

			// Assert:
			AssertSerializedBranch<TPathTraits>(node, numLinks, indexGenerator, expectedMask, result);
		}
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanSerializeBranchWithoutLinks) {
		// Act + Assert:
		AssertCanSerializeBranch<TPathTraits>(0, [](auto) { return 0u; }, 0x0000);
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanSerializeBranchWithLinks) {
		// Act + Assert: mask: 0010'0100'1001'0010 = 0x2492
		AssertCanSerializeBranch<TPathTraits>(5, [](auto i) { return i * 3 + 1u; }, 0x2492);
	}

	// endregion

	// region deserialization - helpers

	namespace {
		template<typename TDestContainer, typename TSourceContainer>
		void Append(TDestContainer& dest, const TSourceContainer& source) {
			dest.insert(dest.end(), source.cbegin(), source.cend());
		}

		auto CreateNodeHeader(uint8_t marker, uint8_t numNibbles, const std::vector<uint8_t>& rawPath) {
			std::vector<uint8_t> data;
			data.push_back(marker);
			data.push_back(numNibbles);
			Append(data, rawPath);
			return data;
		}

		auto CreateSerializedLeaf(const std::vector<uint8_t>& rawPath, size_t size, const Hash256& value) {
			auto data = CreateNodeHeader(0xFF, static_cast<uint8_t>(size), rawPath);
			Append(data, value);
			return data;
		}

		auto CreateSerializedBranch(
				const std::vector<uint8_t>& rawPath,
				size_t size,
				uint16_t linksMask,
				const std::vector<Hash256>& links) {
			auto data = CreateNodeHeader(0x00, static_cast<uint8_t>(size), rawPath);

			std::vector<uint8_t> serializedLinksMask(sizeof(uint16_t));
			reinterpret_cast<uint16_t&>(*serializedLinksMask.data()) = linksMask;
			Append(data, serializedLinksMask);

			for (const auto& link : links)
				Append(data, link);

			return data;
		}

		template<typename TPathTraits>
		auto GenerateValidPath() {
			auto rawPath = test::GenerateRandomDataVector<uint8_t>(TPathTraits::PathSize());
			bool isOdd = 1 == TPathTraits::PathNibbleSize() % 2;
			if (isOdd)
				rawPath[TPathTraits::PathSize() - 1] &= 0xF0;

			return rawPath;
		}

		template<typename TPathTraits>
		auto GenerateInvalidPath() {
			auto rawPath = GenerateValidPath<TPathTraits>();
			rawPath[TPathTraits::PathSize() - 1] |= 0x0F;
			return rawPath;
		}
	}

	// endregion

	// region deserialize - not enough data

	PTSERIALIZER_TRAITS_BASED_TEST(DeserializeFailsIfThereIsNotEnoughData_Leaf) {
		// Arrange:
		auto rawPath = GenerateValidPath<TPathTraits>();
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto data = CreateSerializedLeaf(rawPath, TPathTraits::PathNibbleSize(), value);

		// - drop last byte
		data.resize(data.size() - 1);

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(data), catapult_file_io_error);
	}

	PTSERIALIZER_TRAITS_BASED_TEST(DeserializeFailsIfThereIsNotEnoughData_Branch) {
		// Arrange:
		auto rawPath = GenerateValidPath<TPathTraits>();
		auto data = CreateSerializedBranch(rawPath, TPathTraits::PathNibbleSize(), 0x0000, {});

		// - drop last byte
		data.resize(data.size() - 1);

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(data), catapult_file_io_error);
	}

	// endregion

	// region deserialize - invalid marker

	PTSERIALIZER_TRAITS_BASED_TEST(DeserializeFailsIfMarkerIsInvalid_Leaf) {
		// Arrange:
		auto rawPath = GenerateValidPath<TPathTraits>();
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto data = CreateSerializedLeaf(rawPath, TPathTraits::PathNibbleSize(), value);

		// - change marker
		data[0] = 0x34;

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(data), catapult_invalid_argument);
	}

	// endregion

	// region path deserializing throws

	TEST(TEST_CLASS, DeserializeValueFailsIfPathSizeIsOddAndHasWrongFillerNibble) {
		// Arrange: make sure filler nibble is non-zero
		auto rawPath = GenerateInvalidPath<OddPathTraits>();
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto data = CreateSerializedLeaf(rawPath, OddPathTraits::PathNibbleSize(), value);

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(data), catapult_runtime_error);
	}

	TEST(TEST_CLASS, DeserializeValueDoesNotFailIfPathSizeIsEven) {
		// Arrange:
		auto rawPath = GenerateInvalidPath<EvenPathTraits>();
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto data = CreateSerializedLeaf(rawPath, EvenPathTraits::PathNibbleSize(), value);

		// Act + Assert:
		EXPECT_NO_THROW(Serializer::DeserializeValue(data));
	}

	// endregion

	// region deseralize - valid

	PTSERIALIZER_TRAITS_BASED_TEST(CanDeserializeLeaf) {
		// Arrange:
		auto rawPath = GenerateValidPath<TPathTraits>();
		auto value = test::GenerateRandomData<Hash256_Size>();
		auto data = CreateSerializedLeaf(rawPath, TPathTraits::PathNibbleSize(), value);
		auto path = TreeNodePath(rawPath);

		// Act:
		auto node = Serializer::DeserializeValue(data);

		// Assert:
		EXPECT_TRUE(node.isLeaf());
		const auto& leafNode = node.asLeafNode();
		EXPECT_EQ(TPathTraits::PathNibbleSize(), leafNode.path().size());
		EXPECT_EQ(TPathTraits::PathNibbleSize(), FindFirstDifferenceIndex(path, leafNode.path()));
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
			auto data = CreateSerializedBranch(rawPath, TPathTraits::PathNibbleSize(), linksMask, links);
			auto path = TreeNodePath(rawPath);

			// Act:
			auto node = Serializer::DeserializeValue(data);

			// Assert:
			EXPECT_TRUE(node.isBranch());
			const auto& branchNode = node.asBranchNode();
			EXPECT_EQ(TPathTraits::PathNibbleSize(), branchNode.path().size());
			EXPECT_EQ(TPathTraits::PathNibbleSize(), FindFirstDifferenceIndex(path, branchNode.path()));
			EXPECT_EQ(numLinks, branchNode.numLinks());
			AssertBranchLinks(links, linksMask, branchNode);
		}
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanDeserializeBranchWithoutLinks) {
		// Act + Assert:
		AssertCanDeserializeBranch<TPathTraits>(0, 0x0000);
	}

	PTSERIALIZER_TRAITS_BASED_TEST(CanDeserializeBranchWithLinks) {
		// Act + Assert:
		AssertCanDeserializeBranch<TPathTraits>(4, 0x8421);
	}

	// endregion
}}

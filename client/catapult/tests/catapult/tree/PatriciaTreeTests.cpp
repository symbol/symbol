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

#include "catapult/tree/PatriciaTree.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/Hashers.h"
#include "tests/TestHarness.h"
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace tree {

#define TEST_CLASS PatriciaTreeTests

	// region encoder + data source

	namespace {
		class PassThroughEncoder {
		public:
			using KeyType = uint32_t;
			using ValueType = std::string;

		public:
			static const KeyType& EncodeKey(const KeyType& key) {
				return key;
			}

			static Hash256 EncodeValue(const ValueType& value) {
				Hash256 valueHash;
				crypto::Sha3_256(StringToBuffer(value), valueHash);
				return valueHash;
			}

		private:
			static RawBuffer StringToBuffer(const std::string& str) {
				return { reinterpret_cast<const uint8_t*>(str.data()), str.size() };
			}
		};

		class MemoryDataSource {
		public:
			explicit MemoryDataSource(bool verbose = true) : m_verbose(verbose)
			{}

		public:
			const TreeNode* get(const Hash256& hash) const {
				auto iter = m_nodes.find(hash);
				return m_nodes.cend() != iter ? iter->second.get() : nullptr;
			}

		public:
			void set(const TreeNode& node) {
				if (node.isLeaf())
					set(node.asLeafNode());
				else if (node.isBranch())
					set(node.asBranchNode());
			}

			void set(const BranchTreeNode& node) {
				if (m_verbose) {
					CATAPULT_LOG(debug)
							<< "saving branch node: " << node.path() << ", hash = " << utils::HexFormat(node.hash())
							<< ", #links " << node.numLinks();
				}

				save(node);
			}

			void set(const LeafTreeNode& node) {
				if (m_verbose) {
					CATAPULT_LOG(debug)
						<< "saving leaf node: " << node.path() << ", hash = " << utils::HexFormat(node.hash())
						<< ", value = " << utils::HexFormat(node.value());
				}

				save(node);
			}

		public:
			size_t size() const {
				return m_nodes.size();
			}

			bool contains(const Hash256& hash) const {
				return !!get(hash);
			}

		private:
			template<typename TNode>
			void save(const TNode& node) {
				m_nodes.emplace(node.hash(), std::make_unique<TreeNode>(node));
			}

		private:
			bool m_verbose;
			std::unordered_map<Hash256, std::unique_ptr<TreeNode>, utils::ArrayHasher<Hash256>> m_nodes;
		};
	}

	// endregion

	// region NodeReachabilityChecker

	namespace {
		class NodeReachabilityChecker {
		public:
			explicit NodeReachabilityChecker(const MemoryDataSource& dataSource) : m_dataSource(dataSource)
			{}

		public:
			Hash256 get(const std::string& name) const {
				for (const auto& pair : m_hashNamePairs) {
					if (pair.second == name)
						return pair.first;
				}

				return Hash256();
			}

		public:
			const Hash256& add(const Hash256& hash, const std::string& name) {
				return m_hashNamePairs.emplace(hash, name).first->first;
			}

			const Hash256& addLeaf(const std::vector<uint8_t>& path, const std::string& name) {
				return addLeaf(path, name, name);
			}

			template<typename TValue>
			const Hash256& addLeaf(const std::vector<uint8_t>& path, const TValue& value, const std::string& name) {
				return add(CalculateLeafNodeHash(path, value), name);
			}

			const Hash256& addBranch(
					const std::vector<uint8_t>& path,
					const std::unordered_map<size_t, Hash256>& links,
					const std::string& name) {
				return add(CalculateBranchNodeHash(path, links), name);
			}

		public:
			void checkReachable(const Hash256& hash, std::unordered_set<std::string>&& expectedReachableNames) {
				checkReachable(hash, expectedReachableNames, 0, 0);

				if (expectedReachableNames.empty())
					return;

				std::ostringstream out;
				out << "unable to reach nodes: ";
				for (const auto& name : expectedReachableNames)
					out << name << " ";

				// due to if above, this will always fail with a nice message
				EXPECT_TRUE(m_hashNamePairs.empty()) << out.str();
			}

		private:
			void checkReachable(const Hash256& hash, std::unordered_set<std::string>& expectedReachableNames, size_t level, size_t link) {
				auto message = CreateMessagePrefix(level, link);

				// remove `hash` entry from `expectedReachableNames`
				markUsed(hash, expectedReachableNames, message);

				// retrieve `hash` node from the data source and process it
				ASSERT_TRUE(m_dataSource.contains(hash)) << message;

				const auto& node = *m_dataSource.get(hash);
				if (!node.isBranch())
					return;

				const auto& branchNode = node.asBranchNode();
				for (auto i = 0u; i < 16; ++i) {
					if (branchNode.hasLink(i))
						checkReachable(branchNode.link(i), expectedReachableNames, level + 1, i);
				}
			}

			void markUsed(const Hash256& hash, std::unordered_set<std::string>& expectedReachableNames, const std::string& prefix) {
				auto message = prefix + " unexpected reachable hash: ";

				auto hashNamePairIter = m_hashNamePairs.find(hash);
				ASSERT_TRUE(m_hashNamePairs.cend() != hashNamePairIter) << message << utils::HexFormat(hash);

				auto reachableNameIter = expectedReachableNames.find(hashNamePairIter->second);
				ASSERT_TRUE(expectedReachableNames.cend() != reachableNameIter) << message << hashNamePairIter->second;
				expectedReachableNames.erase(reachableNameIter);
			}

		private:
			static std::string CreateMessagePrefix(size_t level, size_t link) {
				std::ostringstream out;
				out << "(level = " << level << ", link = " << link << ")";
				return out.str();
			}

			static Hash256 CalculateLeafNodeHash(const std::vector<uint8_t>& path, const Hash256& value) {
				Hash256 hash;
				crypto::Sha3_256_Builder builder;
				builder.update(path);
				builder.update(value);
				builder.final(hash);
				return hash;
			}

			static Hash256 CalculateLeafNodeHash(const std::vector<uint8_t>& path, const std::string& value) {
				auto leafHash = CalculateLeafNodeHash(path, PassThroughEncoder::EncodeValue(value));
				CATAPULT_LOG(debug) << "expected leaf hash for '" << value << "' = " << utils::HexFormat(leafHash);
				return leafHash;
			}

			static Hash256 CalculateBranchNodeHash(const std::vector<uint8_t>& path, const std::unordered_map<size_t, Hash256>& links) {
				Hash256 hash;
				crypto::Sha3_256_Builder builder;

				builder.update(path);
				for (auto i = 0u; i < 16; ++i) {
					auto iter = links.find(i);
					builder.update(links.cend() == iter ? Hash256() : iter->second);
				}

				builder.final(hash);
				return hash;
			}

		private:
			const MemoryDataSource& m_dataSource;
			std::map<Hash256, std::string> m_hashNamePairs; // used for nicer error reporting when possible
		};
	}

	// endregion

	// region leaf (lookup) asserts

	namespace {
		template<typename TEncoder, typename TDataSource>
		void AssertLeaves(
				const PatriciaTree<TEncoder, TDataSource>& tree,
				const std::vector<std::pair<typename TEncoder::KeyType, typename TEncoder::ValueType>>& expectedLeaves) {
			// Arrange:
			for (const auto& pair : expectedLeaves) {
				auto expectedHash = TEncoder::EncodeValue(pair.second);

				// Act:
				const auto* pResult = tree.lookup(pair.first);

				// Assert:
				ASSERT_TRUE(!!pResult) << pair.second;
				EXPECT_EQ(expectedHash, *pResult) << pair.second;
			}
		}

		template<typename TEncoder, typename TDataSource>
		void AssertNotLeaves(
				const PatriciaTree<TEncoder, TDataSource>& tree,
				const std::unordered_set<typename TEncoder::KeyType>& expectedUnknownLeaves) {
			// Arrange:
			for (const auto& key : expectedUnknownLeaves) {
				// Act:
				const auto* pResult = tree.lookup(key);

				// Assert:
				EXPECT_FALSE(!!pResult) << utils::HexFormat(key);
			}
		}
	}

	// endregion

	// region constructor

	TEST(TEST_CLASS, TreeHashIsInitiallyZero) {
		// Act:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);

		// Assert:
		EXPECT_EQ(Hash256(), tree.root());
		EXPECT_EQ(0u, dataSource.size());
	}

	// endregion

	// region single value tests

	TEST(TEST_CLASS, CanInsertSingleValue) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);

		// Act:
		tree.set(0x64'6F'67'00, "alpha");

		// Assert:
		NodeReachabilityChecker checker(dataSource);
		checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "alpha");

		EXPECT_EQ(checker.get("alpha"), tree.root());
		EXPECT_EQ(1u, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha" });

		AssertLeaves(tree, { { 0x64'6F'67'00, "alpha" } });
	}

	TEST(TEST_CLASS, CanUndoInsertSingleValue) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'67'00, "alpha");

		// Act:
		auto isUnset = tree.unset(0x64'6F'67'00);

		// Assert:
		EXPECT_TRUE(isUnset);
		EXPECT_EQ(Hash256(), tree.root());
		EXPECT_EQ(1u, dataSource.size());

		AssertNotLeaves(tree, { 0x64'6F'67'00 });
	}

	TEST(TEST_CLASS, CanUpdateSingleValue) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'67'00, "alpha");

		// Sanity:
		NodeReachabilityChecker checker(dataSource);
		checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "alpha");
		checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "beta");

		EXPECT_EQ(checker.get("alpha"), tree.root());
		EXPECT_EQ(1u, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha" });

		// Act:
		tree.set(0x64'6F'67'00, "beta");

		// Assert:
		EXPECT_EQ(checker.get("beta"), tree.root());
		EXPECT_EQ(2u, dataSource.size());
		checker.checkReachable(tree.root(), { "beta" });

		AssertLeaves(tree, { { 0x64'6F'67'00, "beta" } });
	}

	// endregion

	// region single value tests (custom encoder)

	namespace {
		struct CustomEncoder {
			using KeyType = char;
			using ValueType = uint8_t;

			static KeyType EncodeKey(const KeyType& key) {
				return static_cast<char>(10) + key;
			}

			static Hash256 EncodeValue(const ValueType& value) {
				return { { value } };
			}
		};
	}

	TEST(TEST_CLASS, CanInsertSingleValueWithCustomEncoding) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<CustomEncoder, MemoryDataSource> tree(dataSource);

		// Act:
		tree.set('d', 101);

		// Assert:
		// - key is 0x64 ('d') + 0x10 == 0x6E
		NodeReachabilityChecker checker(dataSource);
		checker.addLeaf({ 0x20, 0x6E }, Hash256{ { 101 } }, "root");

		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u, dataSource.size());
		checker.checkReachable(tree.root(), { "root" });

		AssertLeaves(tree, { { 'd', 101 } });
	}

	TEST(TEST_CLASS, CanUndoInsertSingleValueWithCustomEncoding) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<CustomEncoder, MemoryDataSource> tree(dataSource);
		tree.set('d', 101);

		// Act:
		auto isUnset = tree.unset('d');

		// Assert:
		EXPECT_TRUE(isUnset);
		EXPECT_EQ(Hash256(), tree.root());
		EXPECT_EQ(1u, dataSource.size());

		AssertNotLeaves(tree, { 'd' });
	}

	// endregion

	// region multiple values (single branch)

	namespace {
		NodeReachabilityChecker CreateCheckerForCanInsertMultipleValuesWithSamePrefix(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "alpha", "root0");

			// tree 1
			auto alphaHash = checker.addLeaf({ 0x30 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x35 }, "beta");
			checker.addBranch({ 0x00, 0x64, 0x6F, 0x67 }, { { 0, alphaHash }, { 8, betaHash} }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanInsertMultipleValuesWithSamePrefix) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'67'00, "alpha");

		// Sanity:
		auto checker = CreateCheckerForCanInsertMultipleValuesWithSamePrefix(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u, dataSource.size());
		checker.checkReachable(tree.root(), { "root0" });

		// Act:
		tree.set(0x64'6F'67'85, "beta");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root" });

		AssertLeaves(tree, { { 0x64'6F'67'00, "alpha" }, { 0x64'6F'67'85, "beta" } });
	}

	TEST(TEST_CLASS, CanUndoInsertMultipleValuesWithSamePrefix) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'67'00, "alpha");
		tree.set(0x64'6F'67'85, "beta");

		// Act:
		auto isUnset = tree.unset(0x64'6F'67'85);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanInsertMultipleValuesWithSamePrefix(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(4u, dataSource.size());
		checker.checkReachable(tree.root(), { "root0" });

		AssertLeaves(tree, { { 0x64'6F'67'00, "alpha" } });
		AssertNotLeaves(tree, { 0x64'6F'67'85 });
	}

	namespace {
		NodeReachabilityChecker CreateCheckerForCanInsertMultipleValuesWithNoCommonPrefix(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			checker.addLeaf({ 0x20, 0x26, 0x54, 0x32, 0x10 }, "alpha", "root0");

			// tree 1
			auto alphaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "beta");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, betaHash} }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanInsertMultipleValuesWithNoCommonPrefix) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x26'54'32'10, "alpha");

		// Sanity:
		auto checker = CreateCheckerForCanInsertMultipleValuesWithNoCommonPrefix(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u, dataSource.size());
		checker.checkReachable(tree.root(), { "root0" });

		// Act:
		tree.set(0x46'54'32'10, "beta");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root" });

		AssertLeaves(tree, { { 0x26'54'32'10, "alpha" }, { 0x46'54'32'10, "beta" } });
	}

	TEST(TEST_CLASS, CanUndoInsertMultipleValuesWithNoCommonPrefix) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x26'54'32'10, "alpha");
		tree.set(0x46'54'32'10, "beta");

		// Act:
		auto isUnset = tree.unset(0x46'54'32'10);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanInsertMultipleValuesWithNoCommonPrefix(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(4u, dataSource.size());
		checker.checkReachable(tree.root(), { "root0" });

		AssertLeaves(tree, { { 0x26'54'32'10, "alpha" } });
		AssertNotLeaves(tree, { 0x46'54'32'10 });
	}

	// endregion

	// region branch node (root node has no path) modifications

	namespace {
		NodeReachabilityChecker CreateCheckerForCanInsertNewValueIntoBranchNode(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "beta");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, betaHash } }, "root0");

			// tree 1
			auto gammaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "gamma");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, betaHash }, { 9, gammaHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanInsertNewValueIntoBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x26'54'32'10, "alpha");
		tree.set(0x46'54'32'10, "beta");

		// Sanity:
		auto checker = CreateCheckerForCanInsertNewValueIntoBranchNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root0" });

		// Act:
		tree.set(0x96'54'32'10, "gamma");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 2, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "gamma", "root" });

		AssertLeaves(tree, { { 0x26'54'32'10, "alpha" }, { 0x46'54'32'10, "beta" }, { 0x96'54'32'10, "gamma" } });
	}

	TEST(TEST_CLASS, CanUndoInsertNewValueIntoBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x26'54'32'10, "alpha");
		tree.set(0x46'54'32'10, "beta");
		tree.set(0x96'54'32'10, "gamma");

		// Act:
		auto isUnset = tree.unset(0x96'54'32'10);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanInsertNewValueIntoBranchNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(6u, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root0" });

		AssertLeaves(tree, { { 0x26'54'32'10, "alpha" }, { 0x46'54'32'10, "beta" } });
		AssertNotLeaves(tree, { 0x96'54'32'10 });
	}

	namespace {
		NodeReachabilityChecker CreateCheckerForCanUpdateValueInBranchNode(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "beta");
			auto gammaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "gamma");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, betaHash }, { 9, gammaHash } }, "root0");

			// tree 1
			auto zetaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "zeta");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, zetaHash }, { 9, gammaHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanUpdateValueInBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x26'54'32'10, "alpha");
		tree.set(0x46'54'32'10, "beta");
		tree.set(0x96'54'32'10, "gamma");

		// Sanity:
		auto checker = CreateCheckerForCanUpdateValueInBranchNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3 + 2, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "gamma", "root0" });

		// Act:
		tree.set(0x46'54'32'10, "zeta");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 2 + 2, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "zeta", "gamma", "root" });

		AssertLeaves(tree, { { 0x26'54'32'10, "alpha" }, { 0x46'54'32'10, "zeta" }, { 0x96'54'32'10, "gamma" } });
	}

	namespace {
		NodeReachabilityChecker CreateCheckerForCanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch(
				const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x32, 0x10 }, "beta");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, betaHash } }, "root0");

			// tree 1
			auto alphaBetaBranchHash = checker.addBranch({ 0x15, 0x43 }, { { 2, alphaHash }, { 4, betaHash} }, "alpha-beta");
			auto gammaHash = checker.addLeaf({ 0x37, 0x95, 0x92, 0x10 }, "gamma");
			checker.addBranch({ 0x00 }, { { 6, alphaBetaBranchHash }, { 4, gammaHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x65'43'22'10, "alpha");
		tree.set(0x65'43'42'10, "beta");

		// Sanity:
		auto checker = CreateCheckerForCanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root0" });

		// Act:
		tree.set(0x47'95'92'10, "gamma"); // constructed so that first nibble (4) is valid lookup in existing (6543) branch

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "gamma", "alpha-beta", "root" });

		AssertLeaves(tree, { { 0x65'43'22'10, "alpha" }, { 0x65'43'42'10, "beta" }, { 0x47'95'92'10, "gamma" } });
	}

	TEST(TEST_CLASS, CanUndoSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x65'43'22'10, "alpha");
		tree.set(0x65'43'42'10, "beta");
		tree.set(0x47'95'92'10, "gamma");

		// Act:
		auto isUnset = tree.unset(0x47'95'92'10);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(7u, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root0" });

		AssertLeaves(tree, { { 0x65'43'22'10, "alpha" }, { 0x65'43'42'10, "beta" } });
		AssertNotLeaves(tree, { 0x47'95'92'10 });
	}

	// endregion

	// region extension node (root node has path) modifications

	namespace {
		NodeReachabilityChecker CreateCheckerForCanInsertNewValueIntoExtensionBranchNode(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x32, 0x01 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x32, 0x01 }, "beta");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, betaHash } }, "root0");

			// tree 1
			auto gammaHash = checker.addLeaf({ 0x32, 0x01 }, "gamma");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, betaHash }, { 9, gammaHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanInsertNewValueIntoExtensionBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x65'43'22'01, "alpha");
		tree.set(0x65'43'42'01, "beta");

		// Sanity:
		auto checker = CreateCheckerForCanInsertNewValueIntoExtensionBranchNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root0" });

		// Act:
		tree.set(0x65'43'92'01, "gamma");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 2, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "gamma", "root" });

		AssertLeaves(tree, { { 0x65'43'22'01, "alpha" }, { 0x65'43'42'01, "beta" }, { 0x65'43'92'01, "gamma" } });
	}

	TEST(TEST_CLASS, CanUndoInsertNewValueIntoExtensionBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x65'43'22'01, "alpha");
		tree.set(0x65'43'42'01, "beta");
		tree.set(0x65'43'92'01, "gamma");

		// Act:
		auto isUnset = tree.unset(0x65'43'92'01);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanInsertNewValueIntoExtensionBranchNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(6u, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root0" });

		AssertLeaves(tree, { { 0x65'43'22'01, "alpha" }, { 0x65'43'42'01, "beta" } });
		AssertNotLeaves(tree, { 0x65'43'92'01 });
	}

	namespace {
		NodeReachabilityChecker CreateCheckerForCanUpdateValueInExtensionBranchNode(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x32, 0x01 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x32, 0x01 }, "beta");
			auto gammaHash = checker.addLeaf({ 0x32, 0x01 }, "gamma");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, betaHash }, { 9, gammaHash } }, "root0");

			// tree 1
			auto zetaHash = checker.addLeaf({ 0x32, 0x01 }, "zeta");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, zetaHash }, { 9, gammaHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanUpdateValueInExtensionBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x65'43'22'01, "alpha");
		tree.set(0x65'43'42'01, "beta");
		tree.set(0x65'43'92'01, "gamma");

		// Sanity:
		auto checker = CreateCheckerForCanUpdateValueInExtensionBranchNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3 + 2, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "gamma", "root0" });

		// Act:
		tree.set(0x65'43'42'01, "zeta");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 2 + 2, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "zeta", "gamma", "root" });

		AssertLeaves(tree, { { 0x65'43'22'01, "alpha" }, { 0x65'43'42'01, "zeta" }, { 0x65'43'92'01, "gamma" } });
	}

	namespace {
		NodeReachabilityChecker CreateCheckerForCanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch(
				const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x32, 0x10 }, "beta");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 6, betaHash } }, "root0");

			// tree 1
			auto alphaBetaBranchHash = checker.addBranch({ 0x13 }, { { 2, alphaHash }, { 6, betaHash} }, "alpha-beta");
			auto gammaHash = checker.addLeaf({ 0x35, 0x92, 0x10 }, "gamma");
			checker.addBranch({ 0x00, 0x65 }, { { 4, alphaBetaBranchHash }, { 9, gammaHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x65'43'22'10, "alpha");
		tree.set(0x65'43'62'10, "beta");

		// Sanity:
		auto checker = CreateCheckerForCanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root0" });

		// Act:
		tree.set(0x65'95'92'10, "gamma"); // constructed so that first nibble (6) is valid lookup in existing (6543) branch

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "gamma", "alpha-beta", "root" });

		AssertLeaves(tree, { { 0x65'43'22'10, "alpha" }, { 0x65'43'62'10, "beta" }, { 0x65'95'92'10, "gamma" } });
	}

	TEST(TEST_CLASS, CanUndoSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x65'43'22'10, "alpha");
		tree.set(0x65'43'62'10, "beta");
		tree.set(0x65'95'92'10, "gamma");

		// Act:
		auto isUnset = tree.unset(0x65'95'92'10);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(7u, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "beta", "root0" });

		AssertLeaves(tree, { { 0x65'43'22'10, "alpha" }, { 0x65'43'62'10, "beta" } });
		AssertNotLeaves(tree, { 0x65'95'92'10 });
	}

	// endregion

	// region simple puppy tests

	namespace {
		NodeReachabilityChecker CreateCheckerForCanSplitLeafIntoBranchWithTwoLeaves(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto verbHash = checker.addLeaf({ 0x30, 0x00 }, "verb");
			{
				auto puppyHash = checker.addLeaf({ 0x37, 0x00 }, "puppy", "puppy0");
				checker.addBranch({ 0x00, 0x64, 0x6F }, { { 0, verbHash }, { 6, puppyHash } }, "root0");
			}

			// tree 1
			auto puppyHash = checker.addLeaf({ 0x30 }, "puppy");
			auto coinHash = checker.addLeaf({ 0x35 }, "coin");
			auto puppyCoinBranchHash = checker.addBranch({ 0x17 }, { { 0, puppyHash }, { 6, coinHash} }, "puppy-coin");
			checker.addBranch({ 0x00, 0x64, 0x6F }, { { 0, verbHash }, { 6, puppyCoinBranchHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanSplitLeafIntoBranchWithTwoLeaves) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");

		// Sanity:
		auto checker = CreateCheckerForCanSplitLeafIntoBranchWithTwoLeaves(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy0", "root0" });

		// Act:
		tree.set(0x64'6F'67'65, "coin");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 4, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "root" });

		AssertLeaves(tree, { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" } });
	}

	TEST(TEST_CLASS, CanUndoSplitLeafIntoBranchWithTwoLeaves) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");

		// Act:
		auto isUnset = tree.unset(0x64'6F'67'65);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanSplitLeafIntoBranchWithTwoLeaves(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(8u, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy0", "root0" });

		AssertLeaves(tree, { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" } });
		AssertNotLeaves(tree, { 0x64'6F'67'65 });
	}

	namespace {
		NodeReachabilityChecker CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto verbHash = checker.addLeaf({ 0x30, 0x00 }, "verb");
			auto puppyHash = checker.addLeaf({ 0x30 }, "puppy");
			auto coinHash = checker.addLeaf({ 0x35 }, "coin");
			auto puppyCoinBranchHash = checker.addBranch({ 0x17 }, { { 0, puppyHash }, { 6, coinHash} }, "puppy-coin");
			checker.addBranch({ 0x00, 0x64, 0x6F }, { { 0, verbHash }, { 6, puppyCoinBranchHash } }, "root0");

			// tree 1
			auto verbPuppyCoinBranchHash = checker.addBranch(
					{ 0x14, 0x6F },
					{ { 0, verbHash }, { 6, puppyCoinBranchHash } },
					"verb-puppy-coin");
			auto stallionHash = checker.addLeaf({ 0x3A, 0x6F, 0x72, 0x73 }, "stallion");
			checker.addBranch({ 0x00 }, { { 6, verbPuppyCoinBranchHash }, { 7, stallionHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanCreatePuppyTreeWithRootBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");

		// Sanity:
		auto checker = CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3 + 4, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

		// Act:
		tree.set(0x7A'6F'72'73, "stallion");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 4 + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root" });

		AssertLeaves(tree, {
			{ 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" }, { 0x7A'6F'72'73, "stallion" }
		});
	}

	TEST(TEST_CLASS, CanUndoCreatePuppyTreeWithRootBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");
		tree.set(0x7A'6F'72'73, "stallion");

		// Act:
		auto isUnset = tree.unset(0x7A'6F'72'73);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(11u, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

		AssertLeaves(tree, { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" } });
		AssertNotLeaves(tree, { 0x7A'6F'72'73 });
	}

	namespace {
		NodeReachabilityChecker CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(const MemoryDataSource& dataSource) {
			// tree 0
			NodeReachabilityChecker checker(dataSource);
			auto verbHash = checker.addLeaf({ 0x30, 0x00 }, "verb");
			auto puppyHash = checker.addLeaf({ 0x30 }, "puppy");
			auto coinHash = checker.addLeaf({ 0x35 }, "coin");
			auto puppyCoinBranchHash = checker.addBranch({ 0x17 }, { { 0, puppyHash }, { 6, coinHash} }, "puppy-coin");
			checker.addBranch({ 0x00, 0x64, 0x6F }, { { 0, verbHash }, { 6, puppyCoinBranchHash } }, "root0");

			// tree 1
			auto verbPuppyCoinBranchHash = checker.addBranch(
					{ 0x00, 0x6F },
					{ { 0, verbHash }, { 6, puppyCoinBranchHash } },
					"verb-puppy-coin");
			auto stallionHash = checker.addLeaf({ 0x20, 0x6F, 0x72, 0x73 }, "stallion");
			checker.addBranch({ 0x16 }, { { 4, verbPuppyCoinBranchHash }, { 8, stallionHash } }, "root");
			return checker;
		}
	}

	TEST(TEST_CLASS, CanCreatePuppyTreeWithRootExtensionNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");

		// Sanity:
		auto checker = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(1u + 3 + 4, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

		// Act:
		tree.set(0x68'6F'72'73, "stallion");

		// Assert:
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(1u + 3 + 4 + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root" });

		AssertLeaves(tree, {
			{ 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" }, { 0x68'6F'72'73, "stallion" }
		});
	}

	TEST(TEST_CLASS, CanUndoCreatePuppyTreeWithRootExtensionNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");
		tree.set(0x68'6F'72'73, "stallion");

		// Act:
		auto isUnset = tree.unset(0x68'6F'72'73);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(dataSource);
		EXPECT_EQ(checker.get("root0"), tree.root());
		EXPECT_EQ(11u, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

		AssertLeaves(tree, { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" } });
		AssertNotLeaves(tree, { 0x68'6F'72'73 });
	}

	// endregion

	// region unset - not found

	TEST(TEST_CLASS, UnsetHasNoEffectWhenTreeIsEmpty) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);

		// Act:
		EXPECT_FALSE(tree.unset(0x64'6F'67'00));

		// Assert:
		EXPECT_EQ(Hash256(), tree.root());
		EXPECT_EQ(0u, dataSource.size());
	}

	TEST(TEST_CLASS, UnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootLeafNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'67'00, "alpha");

		// Act:
		EXPECT_FALSE(tree.unset(0x64'6F'67'01));

		// Assert:
		NodeReachabilityChecker checker(dataSource);
		checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "alpha");

		EXPECT_EQ(checker.get("alpha"), tree.root());
		EXPECT_EQ(1u, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha" });
	}

	TEST(TEST_CLASS, UnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");
		tree.set(0x7A'6F'72'73, "stallion");

		// Act:
		EXPECT_FALSE(tree.unset(0x54'6F'67'00));
		EXPECT_FALSE(tree.unset(0x64'6F'77'65));
		EXPECT_FALSE(tree.unset(0x64'6F'67'44));

		// Assert:
		auto checker = CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(dataSource);
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(11u, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root" });
	}

	TEST(TEST_CLASS, UnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootExtensionNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");
		tree.set(0x68'6F'72'73, "stallion");

		// Act:
		EXPECT_FALSE(tree.unset(0x54'6F'67'00));
		EXPECT_FALSE(tree.unset(0x65'6F'67'00));
		EXPECT_FALSE(tree.unset(0x64'6F'77'65));
		EXPECT_FALSE(tree.unset(0x64'6F'67'44));

		// Assert:
		auto checker = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(dataSource);
		EXPECT_EQ(checker.get("root"), tree.root());
		EXPECT_EQ(11u, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root" });
	}

	// endregion

	// region unset - new root

	TEST(TEST_CLASS, UnsetCanCreatePreviouslyUnseenRootLeaf) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x26'54'32'10, "alpha");
		tree.set(0x46'54'32'10, "beta");

		// Act:
		auto isUnset = tree.unset(0x26'54'32'10);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanInsertMultipleValuesWithNoCommonPrefix(dataSource);
		checker.addLeaf({ 0x20, 0x46, 0x54, 0x32, 0x10 }, "beta", "root-unset");

		EXPECT_EQ(checker.get("root-unset"), tree.root());
		EXPECT_EQ(4u + 1, dataSource.size());
		checker.checkReachable(tree.root(), { "root-unset" });

		AssertLeaves(tree, { { 0x46'54'32'10, "beta" } });
		AssertNotLeaves(tree, { 0x26'54'32'10 });
	}

	TEST(TEST_CLASS, UnsetCanCreatePreviouslyUnseenRootBranch) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x26'54'32'10, "alpha");
		tree.set(0x46'54'32'10, "beta");
		tree.set(0x96'54'32'10, "gamma");

		// Act:
		auto isUnset = tree.unset(0x46'54'32'10);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanInsertNewValueIntoBranchNode(dataSource);
		checker.addBranch({ 0x00 }, { { 2, checker.get("alpha") }, { 9, checker.get("gamma") } }, "root-unset");

		EXPECT_EQ(checker.get("root-unset"), tree.root());
		EXPECT_EQ(6u + 1, dataSource.size());
		checker.checkReachable(tree.root(), { "alpha", "gamma", "root-unset" });

		AssertLeaves(tree, { { 0x26'54'32'10, "alpha" }, { 0x96'54'32'10, "gamma" } });
		AssertNotLeaves(tree, { 0x46'54'32'10 });
	}

	TEST(TEST_CLASS, UnsetCanCreatePreviouslyUnseenTree) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");
		tree.set(0x68'6F'72'73, "stallion");

		// Act:
		auto isUnset = tree.unset(0x64'6F'67'00);

		// Assert:
		EXPECT_TRUE(isUnset);

		auto checker = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(dataSource);
		auto coinHash = checker.addLeaf({ 0x37, 0x65 }, "coin", "coin-unset");
		auto verbCoinBranchHash = checker.addBranch({ 0x00, 0x6F }, { { 0, checker.get("verb") }, { 6, coinHash } }, "verb-coin");
		checker.addBranch({ 0x16 }, { { 4, verbCoinBranchHash }, { 8, checker.get("stallion") } }, "root-unset");

		EXPECT_EQ(checker.get("root-unset"), tree.root());
		EXPECT_EQ(11u + 3, dataSource.size());
		checker.checkReachable(tree.root(), { "verb", "coin-unset", "verb-coin", "stallion", "root-unset" });

		AssertLeaves(tree, { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'65, "coin" }, { 0x68'6F'72'73, "stallion" } });
		AssertNotLeaves(tree, { 0x64'6F'67'00 });
	}

	// endregion

	// region lookup - not found

	TEST(TEST_CLASS, LookupFailsWhenTreeIsEmpty) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);

		// Act + Assert:
		EXPECT_FALSE(!!tree.lookup(0x64'6F'67'00));
	}

	TEST(TEST_CLASS, LookupFailsWhenKeyIsNotInTreeWithRootLeafNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'67'00, "alpha");

		// Act + Assert:
		EXPECT_FALSE(!!tree.lookup(0x64'6F'67'01));
	}

	TEST(TEST_CLASS, LookupFailsWhenKeyIsNotInTreeWithRootBranchNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");
		tree.set(0x7A'6F'72'73, "stallion");

		// Act + Assert:
		EXPECT_FALSE(!!tree.lookup(0x54'6F'67'00));
		EXPECT_FALSE(!!tree.lookup(0x64'6F'77'65));
		EXPECT_FALSE(!!tree.lookup(0x64'6F'67'44));
	}

	TEST(TEST_CLASS, LookupFailsWhenKeyIsNotInTreeWithRootExtensionNode) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x64'6F'00'00, "verb");
		tree.set(0x64'6F'67'00, "puppy");
		tree.set(0x64'6F'67'65, "coin");
		tree.set(0x68'6F'72'73, "stallion");

		// Act + Assert:
		EXPECT_FALSE(!!tree.lookup(0x54'6F'67'00));
		EXPECT_FALSE(!!tree.lookup(0x65'6F'67'00));
		EXPECT_FALSE(!!tree.lookup(0x64'6F'77'65));
		EXPECT_FALSE(!!tree.lookup(0x64'6F'67'44));
	}

	// endregion

	// region lookup - success

	// a single lookup success test is added here for sanity
	// there are not comprehensive lookup success tests here because lookup is also tested in all set/update/unset tests

	TEST(TEST_CLASS, LookupSucceedsWhenKeyIsInTree) {
		// Arrange:
		MemoryDataSource dataSource;
		PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
		tree.set(0x65'43'22'10, "alpha");
		tree.set(0x65'43'42'10, "beta");
		tree.set(0x47'95'92'10, "gamma");

		// Act:
		auto* pAlphaHash = tree.lookup(0x65'43'22'10);
		auto* pBetaHash = tree.lookup(0x65'43'42'10);
		auto* pGammaHash = tree.lookup(0x47'95'92'10);

		// Assert:
		ASSERT_TRUE(!!pAlphaHash);
		EXPECT_EQ(PassThroughEncoder::EncodeValue("alpha"), *pAlphaHash);

		ASSERT_TRUE(!!pBetaHash);
		EXPECT_EQ(PassThroughEncoder::EncodeValue("beta"), *pBetaHash);

		ASSERT_TRUE(!!pGammaHash);
		EXPECT_EQ(PassThroughEncoder::EncodeValue("gamma"), *pGammaHash);
	}

	// endregion

	// region any order tests

	namespace {
		std::vector<std::pair<uint32_t, std::string>> GetPuppyTreeWithRootExtensionNodePairs() {
			// pairs must be ordered by key for next_permutation
			return {
				{ 0x64'6F'00'00, "verb" },
				{ 0x64'6F'67'00, "puppy" },
				{ 0x64'6F'67'65, "coin" },
				{ 0x68'6F'72'73, "stallion" }
			};
		}
	}

	TEST(TEST_CLASS, CanCreatePuppyTreeWithRootExtensionNode_AnyOrder) {
		// Arrange:
		size_t i = 0u;
		auto pairs = GetPuppyTreeWithRootExtensionNodePairs();
		auto expectedHash = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(MemoryDataSource()).get("root");
		for (; 0 == i || std::next_permutation(pairs.begin(), pairs.end());) {
			MemoryDataSource dataSource(false);
			PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);

			// Act:
			for (const auto& pair : pairs)
				tree.set(pair.first, pair.second);

			// Assert:
			EXPECT_EQ(expectedHash, tree.root()) << "permutation " << i;
			++i;
		};

		// Sanity: 4!
		EXPECT_EQ(24u, i);
	}

	namespace {
		Hash256 CalculateExpectedHashForUndoAnyOrderIteration(
				const std::vector<std::pair<uint32_t, std::string>>& pairs,
				size_t excludedIndex) {
			MemoryDataSource dataSource(false);
			PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
			for (auto i = 0u; i < pairs.size(); ++i) {
				if (excludedIndex != i)
					tree.set(pairs[i].first, pairs[i].second);
			}

			return tree.root();
		}
	}

	TEST(TEST_CLASS, CanUndoPuppyTreeWithRootExtensionNode_AnyOrder) {
		// Arrange:
		size_t i = 0u;
		auto pairs = GetPuppyTreeWithRootExtensionNodePairs();
		for (; 0 == i || std::next_permutation(pairs.begin(), pairs.end());) {
			for (auto j = 0u; j < pairs.size(); ++j) {
				// - calculate the expected hash by applying all values not at index j to a tree
				auto expectedHash = CalculateExpectedHashForUndoAnyOrderIteration(pairs, j);

				// - apply all values to a new tree
				MemoryDataSource dataSource(false);
				PatriciaTree<PassThroughEncoder, MemoryDataSource> tree(dataSource);
				for (const auto& pair : pairs)
					tree.set(pair.first, pair.second);

				// Act: unset the target value (at index j)
				auto isUnset = tree.unset(pairs[j].first);

				// Assert:
				EXPECT_TRUE(isUnset) << "permutation (" << i << ", " << j << ")";
				EXPECT_EQ(expectedHash, tree.root()) << "permutation (" << i << ", " << j << ")";
			}

			++i;
		};

		// Sanity: 4!
		EXPECT_EQ(24u, i);
	}

	// endregion
}}

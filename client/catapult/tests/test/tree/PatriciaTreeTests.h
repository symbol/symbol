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

#pragma once
#include "PassThroughEncoder.h"
#include "catapult/tree/DataSourceVerbosity.h"
#include "catapult/tree/PatriciaTree.h"
#include "tests/TestHarness.h"
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace test {

	// region NodeReachabilityChecker

	/// Reachability checker for patricia tree data source.
	template<typename TDataSource>
	class NodeReachabilityChecker {
	public:
		/// Creates reachability checker around \a dataSource.
		explicit NodeReachabilityChecker(const TDataSource& dataSource) : m_dataSource(dataSource)
		{}

	public:
		/// Gets the hash registered with \a name.
		Hash256 get(const std::string& name) const {
			for (const auto& pair : m_hashNamePairs) {
				if (pair.second == name)
					return pair.first;
			}

			return Hash256();
		}

	public:
		/// Adds leaf \a name with \a path.
		const Hash256& addLeaf(const std::vector<uint8_t>& path, const std::string& name) {
			return addLeaf(path, name, name);
		}

		/// Adds leaf \a name with \a path and \a value.
		template<typename TValue>
		const Hash256& addLeaf(const std::vector<uint8_t>& path, const TValue& value, const std::string& name) {
			return add(CalculateLeafNodeHash(path, value), name);
		}

		/// Adds branch \a name with \a path and \a links.
		const Hash256& addBranch(
				const std::vector<uint8_t>& path,
				const std::unordered_map<size_t, Hash256>& links,
				const std::string& name) {
			return add(CalculateBranchNodeHash(path, links), name);
		}

	public:
		/// Checks if \a expectedReachableNames are reachable from \a hash root in data source.
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
			auto pNode = m_dataSource.get(hash);
			ASSERT_TRUE(!!pNode) << message;

			const auto& node = *pNode;
			if (!node.isBranch())
				return;

			const auto& branchNode = node.asBranchNode();
			for (auto i = 0u; i < 16; ++i) {
				EXPECT_FALSE(!!branchNode.linkedNode(i)) << message << " saved branch tree node has node link at " << i;

				if (branchNode.hasLink(i))
					checkReachable(branchNode.link(i), expectedReachableNames, level + 1, i);
			}
		}

		void markUsed(const Hash256& hash, std::unordered_set<std::string>& expectedReachableNames, const std::string& prefix) {
			auto message = prefix + " unexpected reachable hash: ";

			auto hashNamePairIter = m_hashNamePairs.find(hash);
			ASSERT_TRUE(m_hashNamePairs.cend() != hashNamePairIter) << message << hash;

			auto reachableNameIter = expectedReachableNames.find(hashNamePairIter->second);
			ASSERT_TRUE(expectedReachableNames.cend() != reachableNameIter) << message << hashNamePairIter->second;
			expectedReachableNames.erase(reachableNameIter);
		}

		const Hash256& add(const Hash256& hash, const std::string& name) {
			return m_hashNamePairs.emplace(hash, name).first->first;
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
			CATAPULT_LOG(debug) << "expected leaf hash for '" << value << "' = " << leafHash;
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
		const TDataSource& m_dataSource;
		std::map<Hash256, std::string> m_hashNamePairs; // used for nicer error reporting when possible
	};

	// endregion

	/// Patricia tree test suite.
	template<typename TTraits>
	class PatriciaTreeTests {
	public:
		using DataSource = typename TTraits::DataSourceType;
		using Checker = NodeReachabilityChecker<DataSource>;

	private:
		struct CustomEncoder {
			using KeyType = char;
			using ValueType = uint8_t;

			// EncodeKey should be able to return an unrelated type
			static int16_t EncodeKey(const KeyType& key) {
				return static_cast<int16_t>(10 + key);
			}

			// EncodeValue must return Hash256
			static Hash256 EncodeValue(const ValueType& value) {
				return { { value } };
			}
		};

		template<typename TEncoder>
		class TestContextT {
		public:
			explicit TestContextT(tree::DataSourceVerbosity verbosity = tree::DataSourceVerbosity::Verbose)
					: m_dataSourceWrapper(verbosity)
					, m_tree(m_dataSourceWrapper.dataSource())
			{}

		public:
			auto& dataSource() {
				return m_dataSourceWrapper.dataSource();
			}

			auto& tree() {
				return m_tree;
			}

			void verifyDataSourceSize(size_t expectedSize) {
				// Arrange: save the tree to the data source
				m_tree.saveAll();

				// Act: check the data source
				m_dataSourceWrapper.verifyDataSourceSize(expectedSize);
			}

		public:
			TTraits m_dataSourceWrapper;
			tree::PatriciaTree<TEncoder, DataSource> m_tree;
		};

		using TestContext = TestContextT<PassThroughEncoder>;

	private:
		template<typename TEncoder, typename TDataSource>
		static void AssertLeaves(
				const tree::PatriciaTree<TEncoder, TDataSource>& tree,
				const std::vector<std::pair<typename TEncoder::KeyType, typename TEncoder::ValueType>>& expectedLeaves) {
			// Arrange:
			for (const auto& pair : expectedLeaves) {
				auto expectedHash = TEncoder::EncodeValue(pair.second);

				// Act:
				std::vector<tree::TreeNode> nodePath;
				auto result = tree.lookup(pair.first, nodePath);

				// Assert:
				EXPECT_TRUE(result.second) << pair.second;
				EXPECT_EQ(expectedHash, result.first) << pair.second;
				PatriciaTreeTests<TTraits>::VerifyProof<TEncoder>(tree.root(), pair, nodePath);
			}
		}

		template<typename TEncoder>
		static void VerifyProof(
				const Hash256& rootHash,
				const std::pair<typename TEncoder::KeyType, typename TEncoder::ValueType>& expectedLeaf,
				const std::vector<tree::TreeNode>& nodePath) {
			auto keyPath = tree::TreeNodePath(TEncoder::EncodeKey(expectedLeaf.first));
			auto leafValue = TEncoder::EncodeValue(expectedLeaf.second);
			std::ostringstream messageStream;
			messageStream << "verifying proof for " << expectedLeaf.second << " - " << keyPath;
			auto message = messageStream.str();

			// since this is a proof, the root must be the first node and a leaf must be the last node
			ASSERT_FALSE(nodePath.empty()) << message;
			EXPECT_EQ(rootHash, nodePath.front().hash()) << message;
			EXPECT_TRUE(nodePath.back().isLeaf()) << message;

			// if the path only contains a single node, the matching node must be the root
			if (1 == nodePath.size()) {
				auto expectedLeafHash = tree::LeafTreeNode(keyPath, leafValue).hash();
				EXPECT_EQ(rootHash, expectedLeafHash) << message << " at root leaf";
				EXPECT_EQ(expectedLeafHash, nodePath.back().hash()) << message << " at root leaf";
				return;
			}

			// otherwise, process links from root to last branch
			Hash256 linkedNodeHash;
			for (auto i = 0u; i < nodePath.size() - 1; ++i) {
				EXPECT_TRUE(nodePath[i].isBranch()) << message << " at branch node " << i;

				const auto& branchNode = nodePath[i].asBranchNode();
				if (i > 0)
					EXPECT_EQ(linkedNodeHash, branchNode.hash()) << message << " at branch node " << i;

				auto differenceIndex = FindFirstDifferenceIndex(branchNode.path(), keyPath);
				linkedNodeHash = branchNode.link(keyPath.nibbleAt(differenceIndex));
				keyPath = keyPath.subpath(differenceIndex + 1);
			}

			// check the last (leaf) node
			auto expectedLeafHash = tree::LeafTreeNode(keyPath, leafValue).hash();
			EXPECT_EQ(linkedNodeHash, expectedLeafHash) << message << " at leaf (linked)";
			EXPECT_EQ(expectedLeafHash, nodePath.back().hash()) << message << " at leaf";
		}

		template<typename TEncoder, typename TDataSource>
		static void AssertNotLeaves(
				const tree::PatriciaTree<TEncoder, TDataSource>& tree,
				const std::unordered_set<typename TEncoder::KeyType>& expectedUnknownLeaves) {
			// Arrange:
			for (const auto& key : expectedUnknownLeaves) {
				// Act:
				std::vector<tree::TreeNode> nodePath;
				auto result = tree.lookup(key, nodePath);

				// Assert:
				EXPECT_FALSE(result.second) << key;
				EXPECT_EQ(Hash256(), result.first) << key;
				PatriciaTreeTests<TTraits>::VerifyNoProof<TEncoder>(tree.root(), key, nodePath);
			}
		}

		template<typename TEncoder>
		static void VerifyNoProof(
				const Hash256& rootHash,
				const typename TEncoder::KeyType& expectedUnknownLeaf,
				const std::vector<tree::TreeNode>& nodePath) {
			auto keyPath = tree::TreeNodePath(TEncoder::EncodeKey(expectedUnknownLeaf));
			std::ostringstream messageStream;
			messageStream << "verifying proof for " << keyPath;
			auto message = messageStream.str();

			// if tree is empty, it doesn't contain any nodes
			if (Hash256() == rootHash)
				return;

			// since this is a proof, there must be at least the root node
			ASSERT_FALSE(nodePath.empty()) << message;
			EXPECT_EQ(rootHash, nodePath.front().hash()) << message;

			Hash256 linkedNodeHash;
			for (auto i = 0u; i < nodePath.size(); ++i) {
				// if the last node is a divergent leaf, key can't exist in tree
				if (nodePath.size() - 1 == i && nodePath[i].isLeaf()) {
					EXPECT_NE(keyPath, nodePath[i].path()) << " at divergent leaf";
					return;
				}

				// otherwise, process a branch node
				EXPECT_TRUE(nodePath[i].isBranch()) << message << " at branch node " << i;

				const auto& branchNode = nodePath[i].asBranchNode();
				if (i > 0)
					EXPECT_EQ(linkedNodeHash, branchNode.hash()) << message << " at branch node " << i;

				auto differenceIndex = FindFirstDifferenceIndex(branchNode.path(), keyPath);
				auto linkIndex = keyPath.nibbleAt(differenceIndex);
				if (!branchNode.hasLink(linkIndex))
					return; // there is a gap so key can't exist

				linkedNodeHash = branchNode.link(linkIndex);
				keyPath = keyPath.subpath(differenceIndex + 1);
			}

			FAIL() << message << " has entire key path in tree";
		}

	public:
		// region constructor

		static void AssertTreeHashIsInitiallyZero() {
			// Act:
			TestContext context;

			// Assert:
			EXPECT_EQ(Hash256(), context.tree().root());
			context.verifyDataSourceSize(0);
		}

		// endregion

		// region single value tests

		static void AssertCanInsertSingleValue() {
			// Arrange:
			TestContext context;

			// Act:
			context.tree().set(0x64'6F'67'00, "alpha");

			// Assert:
			Checker checker(context.dataSource());
			checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "alpha");

			EXPECT_EQ(checker.get("alpha"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "alpha" });

			AssertLeaves(context.tree(), { { 0x64'6F'67'00u, "alpha" } });
		}

		static void AssertCanUndoInsertSingleValue() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'67'00, "alpha");

			// Act:
			auto isUnset = context.tree().unset(0x64'6F'67'00);

			// Assert:
			EXPECT_TRUE(isUnset);
			EXPECT_EQ(Hash256(), context.tree().root());
			context.verifyDataSourceSize(0);

			AssertNotLeaves(context.tree(), { 0x64'6F'67'00 });
		}

		static void AssertCanUpdateSingleValue() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'67'00, "alpha");

			// Sanity:
			Checker checker(context.dataSource());
			checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "alpha");
			checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "beta");

			EXPECT_EQ(checker.get("alpha"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "alpha" });

			// Act:
			context.tree().set(0x64'6F'67'00, "beta");

			// Assert:
			EXPECT_EQ(checker.get("beta"), context.tree().root());
			context.verifyDataSourceSize(2);
			checker.checkReachable(context.tree().root(), { "beta" });

			AssertLeaves(context.tree(), { { 0x64'6F'67'00u, "beta" } });
		}

		// endregion

		// region single value tests (custom encoder)

		static void AssertCanInsertSingleValueWithCustomEncoding() {
			// Arrange:
			TestContextT<CustomEncoder> context;

			// Act:
			context.tree().set('d', 101);

			// Assert:
			// - key is 0x64 ('d') + 0x10 == 0x6E
			Checker checker(context.dataSource());
			checker.addLeaf({ 0x20, 0x00, 0x6E }, Hash256{ { 101 } }, "root");

			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "root" });

			AssertLeaves(context.tree(), { { 'd', static_cast<uint8_t>(101) } });
		}

		static void AssertCanUndoInsertSingleValueWithCustomEncoding() {
			// Arrange:
			TestContextT<CustomEncoder> context;
			context.tree().set('d', 101);

			// Act:
			auto isUnset = context.tree().unset('d');

			// Assert:
			EXPECT_TRUE(isUnset);
			EXPECT_EQ(Hash256(), context.tree().root());
			context.verifyDataSourceSize(0);

			AssertNotLeaves(context.tree(), { 'd' });
		}

		// endregion

		// region multiple values (single branch)

	private:
		static Checker CreateCheckerForCanInsertMultipleValuesWithSamePrefix(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
			checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "alpha", "root0");

			// tree 1
			auto alphaHash = checker.addLeaf({ 0x30 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x35 }, "beta");
			checker.addBranch({ 0x00, 0x64, 0x6F, 0x67 }, { { 0, alphaHash }, { 8, betaHash} }, "root");
			return checker;
		}

	public:
		static void AssertCanInsertMultipleValuesWithSamePrefix() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'67'00, "alpha");

			// Sanity:
			auto checker = CreateCheckerForCanInsertMultipleValuesWithSamePrefix(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "root0" });

			// Act:
			context.tree().set(0x64'6F'67'85, "beta");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(1 + 3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root" });

			AssertLeaves(context.tree(), { { 0x64'6F'67'00, "alpha" }, { 0x64'6F'67'85, "beta" } });
		}

		static void AssertCanUndoInsertMultipleValuesWithSamePrefix() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'67'00, "alpha");
			context.tree().set(0x64'6F'67'85, "beta");

			// Act:
			auto isUnset = context.tree().unset(0x64'6F'67'85);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanInsertMultipleValuesWithSamePrefix(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "root0" });

			AssertLeaves(context.tree(), { { 0x64'6F'67'00, "alpha" } });
			AssertNotLeaves(context.tree(), { 0x64'6F'67'85 });
		}

	private:
		static Checker CreateCheckerForCanInsertMultipleValuesWithNoCommonPrefix(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
			checker.addLeaf({ 0x20, 0x26, 0x54, 0x32, 0x10 }, "alpha", "root0");

			// tree 1
			auto alphaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "beta");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, betaHash} }, "root");
			return checker;
		}

	public:
		static void AssertCanInsertMultipleValuesWithNoCommonPrefix() {
			// Arrange:
			TestContext context;
			context.tree().set(0x26'54'32'10, "alpha");

			// Sanity:
			auto checker = CreateCheckerForCanInsertMultipleValuesWithNoCommonPrefix(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "root0" });

			// Act:
			context.tree().set(0x46'54'32'10, "beta");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(1 + 3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root" });

			AssertLeaves(context.tree(), { { 0x26'54'32'10, "alpha" }, { 0x46'54'32'10, "beta" } });
		}

		static void AssertCanUndoInsertMultipleValuesWithNoCommonPrefix() {
			// Arrange:
			TestContext context;
			context.tree().set(0x26'54'32'10, "alpha");
			context.tree().set(0x46'54'32'10, "beta");

			// Act:
			auto isUnset = context.tree().unset(0x46'54'32'10);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanInsertMultipleValuesWithNoCommonPrefix(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "root0" });

			AssertLeaves(context.tree(), { { 0x26'54'32'10, "alpha" } });
			AssertNotLeaves(context.tree(), { 0x46'54'32'10 });
		}

		// endregion

		// region branch node (root node has no path) modifications

	private:
		static Checker CreateCheckerForCanInsertNewValueIntoBranchNode(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "beta");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, betaHash } }, "root0");

			// tree 1
			auto gammaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "gamma");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, betaHash }, { 9, gammaHash } }, "root");
			return checker;
		}

	public:
		static void AssertCanInsertNewValueIntoBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x26'54'32'10, "alpha");
			context.tree().set(0x46'54'32'10, "beta");

			// Sanity:
			auto checker = CreateCheckerForCanInsertNewValueIntoBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root0" });

			// Act:
			context.tree().set(0x96'54'32'10, "gamma");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(3 + 2);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "gamma", "root" });

			AssertLeaves(context.tree(), { { 0x26'54'32'10, "alpha" }, { 0x46'54'32'10, "beta" }, { 0x96'54'32'10, "gamma" } });
		}

		static void AssertCanUndoInsertNewValueIntoBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x26'54'32'10, "alpha");
			context.tree().set(0x46'54'32'10, "beta");
			context.tree().set(0x96'54'32'10, "gamma");

			// Act:
			auto isUnset = context.tree().unset(0x96'54'32'10);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanInsertNewValueIntoBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root0" });

			AssertLeaves(context.tree(), { { 0x26'54'32'10, "alpha" }, { 0x46'54'32'10, "beta" } });
			AssertNotLeaves(context.tree(), { 0x96'54'32'10 });
		}

	private:
		static Checker CreateCheckerForCanUpdateValueInBranchNode(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "beta");
			auto gammaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "gamma");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, betaHash }, { 9, gammaHash } }, "root0");

			// tree 1
			auto zetaHash = checker.addLeaf({ 0x36, 0x54, 0x32, 0x10 }, "zeta");
			checker.addBranch({ 0x00 }, { { 2, alphaHash }, { 4, zetaHash }, { 9, gammaHash } }, "root");
			return checker;
		}

	public:
		static void AssertCanUpdateValueInBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x26'54'32'10, "alpha");
			context.tree().set(0x46'54'32'10, "beta");
			context.tree().set(0x96'54'32'10, "gamma");

			// Sanity:
			auto checker = CreateCheckerForCanUpdateValueInBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(4);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "gamma", "root0" });

			// Act:
			context.tree().set(0x46'54'32'10, "zeta");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(4 + 2);
			checker.checkReachable(context.tree().root(), { "alpha", "zeta", "gamma", "root" });

			AssertLeaves(context.tree(), { { 0x26'54'32'10, "alpha" }, { 0x46'54'32'10, "zeta" }, { 0x96'54'32'10, "gamma" } });
		}

	private:
		static Checker CreateCheckerForCanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x32, 0x10 }, "beta");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, betaHash } }, "root0");

			// tree 1
			auto alphaBetaBranchHash = checker.addBranch({ 0x15, 0x43 }, { { 2, alphaHash }, { 4, betaHash} }, "alpha-beta");
			auto gammaHash = checker.addLeaf({ 0x37, 0x95, 0x92, 0x10 }, "gamma");
			checker.addBranch({ 0x00 }, { { 6, alphaBetaBranchHash }, { 4, gammaHash } }, "root");
			return checker;
		}

	public:
		static void AssertCanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'10, "alpha");
			context.tree().set(0x65'43'42'10, "beta");

			// Sanity:
			auto checker = CreateCheckerForCanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root0" });

			// Act:
			context.tree().set(0x47'95'92'10, "gamma"); // constructed so that first nibble (4) is valid lookup in existing (6543) branch

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(3 + 3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "gamma", "alpha-beta", "root" });

			AssertLeaves(context.tree(), { { 0x65'43'22'10, "alpha" }, { 0x65'43'42'10, "beta" }, { 0x47'95'92'10, "gamma" } });
		}

		static void AssertCanUndoSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'10, "alpha");
			context.tree().set(0x65'43'42'10, "beta");
			context.tree().set(0x47'95'92'10, "gamma");

			// Act:
			auto isUnset = context.tree().unset(0x47'95'92'10);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root0" });

			AssertLeaves(context.tree(), { { 0x65'43'22'10, "alpha" }, { 0x65'43'42'10, "beta" } });
			AssertNotLeaves(context.tree(), { 0x47'95'92'10 });
		}

		// endregion

		// region extension node (root node has path) modifications

	private:
		static Checker CreateCheckerForCanInsertNewValueIntoExtensionBranchNode(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x32, 0x01 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x32, 0x01 }, "beta");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, betaHash } }, "root0");

			// tree 1
			auto gammaHash = checker.addLeaf({ 0x32, 0x01 }, "gamma");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, betaHash }, { 9, gammaHash } }, "root");
			return checker;
		}

	public:
		static void AssertCanInsertNewValueIntoExtensionBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'01, "alpha");
			context.tree().set(0x65'43'42'01, "beta");

			// Sanity:
			auto checker = CreateCheckerForCanInsertNewValueIntoExtensionBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root0" });

			// Act:
			context.tree().set(0x65'43'92'01, "gamma");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(3 + 2);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "gamma", "root" });

			AssertLeaves(context.tree(), { { 0x65'43'22'01, "alpha" }, { 0x65'43'42'01, "beta" }, { 0x65'43'92'01, "gamma" } });
		}

		static void AssertCanUndoInsertNewValueIntoExtensionBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'01, "alpha");
			context.tree().set(0x65'43'42'01, "beta");
			context.tree().set(0x65'43'92'01, "gamma");

			// Act:
			auto isUnset = context.tree().unset(0x65'43'92'01);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanInsertNewValueIntoExtensionBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root0" });

			AssertLeaves(context.tree(), { { 0x65'43'22'01, "alpha" }, { 0x65'43'42'01, "beta" } });
			AssertNotLeaves(context.tree(), { 0x65'43'92'01 });
		}

	private:
		static Checker CreateCheckerForCanUpdateValueInExtensionBranchNode(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x32, 0x01 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x32, 0x01 }, "beta");
			auto gammaHash = checker.addLeaf({ 0x32, 0x01 }, "gamma");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, betaHash }, { 9, gammaHash } }, "root0");

			// tree 1
			auto zetaHash = checker.addLeaf({ 0x32, 0x01 }, "zeta");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 4, zetaHash }, { 9, gammaHash } }, "root");
			return checker;
		}

	public:
		static void AssertCanUpdateValueInExtensionBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'01, "alpha");
			context.tree().set(0x65'43'42'01, "beta");
			context.tree().set(0x65'43'92'01, "gamma");

			// Sanity:
			auto checker = CreateCheckerForCanUpdateValueInExtensionBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(4);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "gamma", "root0" });

			// Act:
			context.tree().set(0x65'43'42'01, "zeta");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(4 + 2);
			checker.checkReachable(context.tree().root(), { "alpha", "zeta", "gamma", "root" });

			AssertLeaves(context.tree(), { { 0x65'43'22'01, "alpha" }, { 0x65'43'42'01, "zeta" }, { 0x65'43'92'01, "gamma" } });
		}

	private:
		static Checker CreateCheckerForCanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
			auto alphaHash = checker.addLeaf({ 0x32, 0x10 }, "alpha");
			auto betaHash = checker.addLeaf({ 0x32, 0x10 }, "beta");
			checker.addBranch({ 0x00, 0x65, 0x43 }, { { 2, alphaHash }, { 6, betaHash } }, "root0");

			// tree 1
			auto alphaBetaBranchHash = checker.addBranch({ 0x13 }, { { 2, alphaHash }, { 6, betaHash} }, "alpha-beta");
			auto gammaHash = checker.addLeaf({ 0x35, 0x92, 0x10 }, "gamma");
			checker.addBranch({ 0x00, 0x65 }, { { 4, alphaBetaBranchHash }, { 9, gammaHash } }, "root");
			return checker;
		}

	public:
		static void AssertCanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'10, "alpha");
			context.tree().set(0x65'43'62'10, "beta");

			// Sanity:
			auto checker = CreateCheckerForCanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root0" });

			// Act:
			context.tree().set(0x65'95'92'10, "gamma"); // constructed so that first nibble (6) is valid lookup in existing (6543) branch

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(3 + 3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "gamma", "alpha-beta", "root" });

			AssertLeaves(context.tree(), { { 0x65'43'22'10, "alpha" }, { 0x65'43'62'10, "beta" }, { 0x65'95'92'10, "gamma" } });
		}

		static void AssertCanUndoSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'10, "alpha");
			context.tree().set(0x65'43'62'10, "beta");
			context.tree().set(0x65'95'92'10, "gamma");

			// Act:
			auto isUnset = context.tree().unset(0x65'95'92'10);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "beta", "root0" });

			AssertLeaves(context.tree(), { { 0x65'43'22'10, "alpha" }, { 0x65'43'62'10, "beta" } });
			AssertNotLeaves(context.tree(), { 0x65'95'92'10 });
		}

		// endregion

		// region simple puppy tests

	private:
		static Checker CreateCheckerForCanSplitLeafIntoBranchWithTwoLeaves(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
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

	public:
		static void AssertCanSplitLeafIntoBranchWithTwoLeaves() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");

			// Sanity:
			auto checker = CreateCheckerForCanSplitLeafIntoBranchWithTwoLeaves(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "verb", "puppy0", "root0" });

			// Act:
			context.tree().set(0x64'6F'67'65, "coin");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(3 + 4);
			checker.checkReachable(context.tree().root(), { "verb", "puppy", "coin", "puppy-coin", "root" });

			AssertLeaves(context.tree(), { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" } });
		}

		static void AssertCanUndoSplitLeafIntoBranchWithTwoLeaves() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");

			// Act:
			auto isUnset = context.tree().unset(0x64'6F'67'65);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanSplitLeafIntoBranchWithTwoLeaves(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "verb", "puppy0", "root0" });

			AssertLeaves(context.tree(), { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" } });
			AssertNotLeaves(context.tree(), { 0x64'6F'67'65 });
		}

	private:
		static Checker CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
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

	public:
		static void AssertCanCreatePuppyTreeWithRootBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");

			// Sanity:
			auto checker = CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(5);
			checker.checkReachable(context.tree().root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

			// Act:
			context.tree().set(0x7A'6F'72'73, "stallion");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(5 + 3);
			checker.checkReachable(context.tree().root(), {
				"verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root"
			});

			AssertLeaves(context.tree(), {
				{ 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" }, { 0x7A'6F'72'73, "stallion" }
			});
		}

		static void AssertCanUndoCreatePuppyTreeWithRootBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");
			context.tree().set(0x7A'6F'72'73, "stallion");

			// Act:
			auto isUnset = context.tree().unset(0x7A'6F'72'73);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(5);
			checker.checkReachable(context.tree().root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

			AssertLeaves(context.tree(), { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" } });
			AssertNotLeaves(context.tree(), { 0x7A'6F'72'73 });
		}

	private:
		static Checker CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(const DataSource& dataSource) {
			// tree 0
			Checker checker(dataSource);
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

	public:
		static void AssertCanCreatePuppyTreeWithRootExtensionNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");

			// Sanity:
			auto checker = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(5);
			checker.checkReachable(context.tree().root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

			// Act:
			context.tree().set(0x68'6F'72'73, "stallion");

			// Assert:
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(5 + 3);
			checker.checkReachable(context.tree().root(), {
				"verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root"
			});

			AssertLeaves(context.tree(), {
				{ 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" }, { 0x68'6F'72'73, "stallion" }
			});
		}

		static void AssertCanUndoCreatePuppyTreeWithRootExtensionNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");
			context.tree().set(0x68'6F'72'73, "stallion");

			// Act:
			auto isUnset = context.tree().unset(0x68'6F'72'73);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(context.dataSource());
			EXPECT_EQ(checker.get("root0"), context.tree().root());
			context.verifyDataSourceSize(5);
			checker.checkReachable(context.tree().root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

			AssertLeaves(context.tree(), { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" } });
			AssertNotLeaves(context.tree(), { 0x68'6F'72'73 });
		}

		// endregion

		// region unset - not found

		static void AssertUnsetHasNoEffectWhenTreeIsEmpty() {
			// Arrange:
			TestContext context;

			// Act:
			EXPECT_FALSE(context.tree().unset(0x64'6F'67'00));

			// Assert:
			EXPECT_EQ(Hash256(), context.tree().root());
			context.verifyDataSourceSize(0);
		}

		static void AssertUnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootLeafNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'67'00, "alpha");

			// Act:
			EXPECT_FALSE(context.tree().unset(0x64'6F'67'01));

			// Assert:
			Checker checker(context.dataSource());
			checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x00 }, "alpha");

			EXPECT_EQ(checker.get("alpha"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "alpha" });
		}

		static void AssertUnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");
			context.tree().set(0x7A'6F'72'73, "stallion");

			// Act:
			EXPECT_FALSE(context.tree().unset(0x54'6F'67'00));
			EXPECT_FALSE(context.tree().unset(0x64'6F'77'65));
			EXPECT_FALSE(context.tree().unset(0x64'6F'67'44));

			// Assert:
			auto checker = CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(context.dataSource());
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(7);
			checker.checkReachable(context.tree().root(), {
				"verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root"
			});
		}

		static void AssertUnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootExtensionNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");
			context.tree().set(0x68'6F'72'73, "stallion");

			// Act:
			EXPECT_FALSE(context.tree().unset(0x54'6F'67'00));
			EXPECT_FALSE(context.tree().unset(0x65'6F'67'00));
			EXPECT_FALSE(context.tree().unset(0x64'6F'77'65));
			EXPECT_FALSE(context.tree().unset(0x64'6F'67'44));

			// Assert:
			auto checker = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(context.dataSource());
			EXPECT_EQ(checker.get("root"), context.tree().root());
			context.verifyDataSourceSize(7);
			checker.checkReachable(context.tree().root(), {
				"verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root"
			});
		}

		// endregion

		// region unset - new root

		static void AssertUnsetCanCreatePreviouslyUnseenRootLeaf() {
			// Arrange:
			TestContext context;
			context.tree().set(0x26'54'32'10, "alpha");
			context.tree().set(0x46'54'32'10, "beta");

			// Act:
			auto isUnset = context.tree().unset(0x26'54'32'10);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanInsertMultipleValuesWithNoCommonPrefix(context.dataSource());
			checker.addLeaf({ 0x20, 0x46, 0x54, 0x32, 0x10 }, "beta", "root-unset");

			EXPECT_EQ(checker.get("root-unset"), context.tree().root());
			context.verifyDataSourceSize(1);
			checker.checkReachable(context.tree().root(), { "root-unset" });

			AssertLeaves(context.tree(), { { 0x46'54'32'10, "beta" } });
			AssertNotLeaves(context.tree(), { 0x26'54'32'10 });
		}

		static void AssertUnsetCanCreatePreviouslyUnseenRootBranch() {
			// Arrange:
			TestContext context;
			context.tree().set(0x26'54'32'10, "alpha");
			context.tree().set(0x46'54'32'10, "beta");
			context.tree().set(0x96'54'32'10, "gamma");

			// Act:
			auto isUnset = context.tree().unset(0x46'54'32'10);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanInsertNewValueIntoBranchNode(context.dataSource());
			checker.addBranch({ 0x00 }, { { 2, checker.get("alpha") }, { 9, checker.get("gamma") } }, "root-unset");

			EXPECT_EQ(checker.get("root-unset"), context.tree().root());
			context.verifyDataSourceSize(3);
			checker.checkReachable(context.tree().root(), { "alpha", "gamma", "root-unset" });

			AssertLeaves(context.tree(), { { 0x26'54'32'10, "alpha" }, { 0x96'54'32'10, "gamma" } });
			AssertNotLeaves(context.tree(), { 0x46'54'32'10 });
		}

		static void AssertUnsetCanCreatePreviouslyUnseenTree() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");
			context.tree().set(0x68'6F'72'73, "stallion");

			// Act:
			auto isUnset = context.tree().unset(0x64'6F'67'00);

			// Assert:
			EXPECT_TRUE(isUnset);

			auto checker = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(context.dataSource());
			auto coinHash = checker.addLeaf({ 0x37, 0x65 }, "coin", "coin-unset");
			auto verbCoinBranchHash = checker.addBranch({ 0x00, 0x6F }, { { 0, checker.get("verb") }, { 6, coinHash } }, "verb-coin");
			checker.addBranch({ 0x16 }, { { 4, verbCoinBranchHash }, { 8, checker.get("stallion") } }, "root-unset");

			EXPECT_EQ(checker.get("root-unset"), context.tree().root());
			context.verifyDataSourceSize(5);
			checker.checkReachable(context.tree().root(), { "verb", "coin-unset", "verb-coin", "stallion", "root-unset" });

			AssertLeaves(context.tree(), { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'65, "coin" }, { 0x68'6F'72'73, "stallion" } });
			AssertNotLeaves(context.tree(), { 0x64'6F'67'00 });
		}

		// endregion

		// region lookup - not found

	private:
		template<typename TTree>
		static void AssertNotFound(const TTree& tree, uint32_t value) {
			// Act:
			std::vector<tree::TreeNode> nodePath;
			auto result = tree.lookup(value, nodePath);

			// Assert:
			EXPECT_FALSE(result.second) << value;
			EXPECT_EQ(Hash256(), result.first) << value;
			PatriciaTreeTests<TTraits>::VerifyNoProof<PassThroughEncoder>(tree.root(), value, nodePath);
		}

	public:
		static void AssertLookupFailsWhenTreeIsEmpty() {
			// Arrange:
			TestContext context;

			// Act + Assert:
			AssertNotFound(context.tree(), 0x64'6F'67'00);
		}

		static void AssertLookupFailsWhenKeyIsNotInTreeWithRootLeafNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'67'00, "alpha");

			// Act + Assert:
			AssertNotFound(context.tree(), 0x64'6F'67'01);
		}

		static void AssertLookupFailsWhenKeyIsNotInTreeWithRootBranchNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");
			context.tree().set(0x7A'6F'72'73, "stallion");

			// Act + Assert:
			AssertNotFound(context.tree(), 0x54'6F'67'00);
			AssertNotFound(context.tree(), 0x64'6F'77'65);
			AssertNotFound(context.tree(), 0x64'6F'67'44);
		}

		static void AssertLookupFailsWhenKeyIsNotInTreeWithRootExtensionNode() {
			// Arrange:
			TestContext context;
			context.tree().set(0x64'6F'00'00, "verb");
			context.tree().set(0x64'6F'67'00, "puppy");
			context.tree().set(0x64'6F'67'65, "coin");
			context.tree().set(0x68'6F'72'73, "stallion");

			// Act + Assert:
			AssertNotFound(context.tree(), 0x54'6F'67'00);
			AssertNotFound(context.tree(), 0x65'6F'67'00);
			AssertNotFound(context.tree(), 0x64'6F'77'65);
			AssertNotFound(context.tree(), 0x64'6F'67'44);
		}

		// endregion

		// region lookup - success

		// a small number of lookup success tests are added here for sanity
		// there are not comprehensive lookup success tests here because lookup is also tested in all set/update/unset tests

		static void AssertLookupSucceedsWhenKeyIsTreeRoot() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'10, "alpha");

			// Act:
			std::vector<tree::TreeNode> alphaNodePath;
			auto alphaResult = context.tree().lookup(0x65'43'22'10, alphaNodePath);

			// Assert:
			EXPECT_TRUE(alphaResult.second);
			EXPECT_EQ(PassThroughEncoder::EncodeValue("alpha"), alphaResult.first);
			PatriciaTreeTests<TTraits>::VerifyProof<PassThroughEncoder>(context.tree().root(), { 0x65'43'22'10, "alpha" }, alphaNodePath);
		}

		static void AssertLookupSucceedsWhenKeyIsInTree() {
			// Arrange:
			TestContext context;
			context.tree().set(0x65'43'22'10, "alpha");
			context.tree().set(0x65'43'42'10, "beta");
			context.tree().set(0x47'95'92'10, "gamma");

			// Act:
			std::vector<tree::TreeNode> alphaNodePath;
			auto alphaResult = context.tree().lookup(0x65'43'22'10, alphaNodePath);

			std::vector<tree::TreeNode> betaNodePath;
			auto betaResult = context.tree().lookup(0x65'43'42'10, betaNodePath);

			std::vector<tree::TreeNode> gammaNodePath;
			auto gammaResult = context.tree().lookup(0x47'95'92'10, gammaNodePath);
			const auto& rootHash = context.tree().root();

			// Assert:
			EXPECT_TRUE(alphaResult.second);
			EXPECT_EQ(PassThroughEncoder::EncodeValue("alpha"), alphaResult.first);
			PatriciaTreeTests<TTraits>::VerifyProof<PassThroughEncoder>(rootHash, { 0x65'43'22'10, "alpha" }, alphaNodePath);

			EXPECT_TRUE(betaResult.second);
			EXPECT_EQ(PassThroughEncoder::EncodeValue("beta"), betaResult.first);
			PatriciaTreeTests<TTraits>::VerifyProof<PassThroughEncoder>(rootHash, { 0x65'43'42'10, "beta" }, betaNodePath);

			EXPECT_TRUE(gammaResult.second);
			EXPECT_EQ(PassThroughEncoder::EncodeValue("gamma"), gammaResult.first);
			PatriciaTreeTests<TTraits>::VerifyProof<PassThroughEncoder>(rootHash, { 0x47'95'92'10, "gamma" }, gammaNodePath);
		}

		// endregion

		// region any order tests

	private:
		static std::vector<std::pair<uint32_t, std::string>> GetPuppyTreeWithRootExtensionNodePairs() {
			// pairs must be ordered by key for next_permutation
			return {
				{ 0x64'6F'00'00, "verb" },
				{ 0x64'6F'67'00, "puppy" },
				{ 0x64'6F'67'65, "coin" },
				{ 0x68'6F'72'73, "stallion" }
			};
		}

	public:
		static void AssertCanCreatePuppyTreeWithRootExtensionNode_AnyOrder() {
			// Arrange:
			size_t i = 0u;
			auto pairs = GetPuppyTreeWithRootExtensionNodePairs();
			Hash256 expectedHash;
			{
				TestContext context(tree::DataSourceVerbosity::Off);
				expectedHash = CreateCheckerForCanCreatePuppyTreeWithRootExtensionNode(context.dataSource()).get("root");
			}

			for (; 0 == i || std::next_permutation(pairs.begin(), pairs.end());) {
				TestContext context(tree::DataSourceVerbosity::Off);

				// Act:
				for (const auto& pair : pairs)
					context.tree().set(pair.first, pair.second);

				// Assert:
				EXPECT_EQ(expectedHash, context.tree().root()) << "permutation " << i;
				++i;
			}

			// Sanity: 4!
			EXPECT_EQ(24u, i);
		}

	private:
		static Hash256 CalculateExpectedHashForUndoAnyOrderIteration(
				const std::vector<std::pair<uint32_t, std::string>>& pairs,
				size_t excludedIndex) {
			TestContext context(tree::DataSourceVerbosity::Off);
			for (auto i = 0u; i < pairs.size(); ++i) {
				if (excludedIndex != i)
					context.tree().set(pairs[i].first, pairs[i].second);
			}

			return context.tree().root();
		}

	public:
		static void AssertCanUndoPuppyTreeWithRootExtensionNode_AnyOrder() {
			// Arrange:
			size_t i = 0u;
			auto pairs = GetPuppyTreeWithRootExtensionNodePairs();
			for (; 0 == i || std::next_permutation(pairs.begin(), pairs.end());) {
				for (auto j = 0u; j < pairs.size(); ++j) {
					// - calculate the expected hash by applying all values not at index j to a tree
					auto expectedHash = CalculateExpectedHashForUndoAnyOrderIteration(pairs, j);

					// - apply all values to a new tree
					TestContext context(tree::DataSourceVerbosity::Off);
					for (const auto& pair : pairs)
						context.tree().set(pair.first, pair.second);

					// Act: unset the target value (at index j)
					auto isUnset = context.tree().unset(pairs[j].first);

					// Assert:
					EXPECT_TRUE(isUnset) << "permutation (" << i << ", " << j << ")";
					EXPECT_EQ(expectedHash, context.tree().root()) << "permutation (" << i << ", " << j << ")";
				}

				++i;
			}

			// Sanity: 4!
			EXPECT_EQ(24u, i);
		}

		// endregion

		// region tryLoad

	private:
		template<typename TAction>
		static void RunLoadTest(TAction action) {
			// Arrange:
			TestContext context;

			auto checker = CreateCheckerForCanCreatePuppyTreeWithRootBranchNode(context.dataSource());
			{
				// - write a tree to the data source and destroy the tree
				tree::PatriciaTree<PassThroughEncoder, DataSource> tree(context.dataSource());
				tree.set(0x64'6F'00'00, "verb");
				tree.set(0x64'6F'67'00, "puppy");
				tree.set(0x64'6F'67'65, "coin");
				tree.saveAll(); // root0

				tree.set(0x7A'6F'72'73, "stallion");
				tree.saveAll(); // root

				// Sanity:
				EXPECT_EQ(checker.get("root"), tree.root());
				context.verifyDataSourceSize(5 + 3);
			}

			// Act:
			tree::PatriciaTree<PassThroughEncoder, DataSource> tree(context.dataSource());
			action(tree, std::move(checker));
		}

		static void AssertTreeLoadRoot0(const tree::PatriciaTree<PassThroughEncoder, DataSource>& tree, Checker&& checker) {
			EXPECT_EQ(checker.get("root0"), tree.root());
			checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "root0" });

			AssertLeaves(tree, { { 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" } });
		}

		static void AssertTreeLoadRoot(const tree::PatriciaTree<PassThroughEncoder, DataSource>& tree, Checker&& checker) {
			EXPECT_EQ(checker.get("root"), tree.root());
			checker.checkReachable(tree.root(), { "verb", "puppy", "coin", "puppy-coin", "verb-puppy-coin", "stallion", "root" });

			AssertLeaves(tree, {
				{ 0x64'6F'00'00, "verb" }, { 0x64'6F'67'00, "puppy" }, { 0x64'6F'67'65, "coin" }, { 0x7A'6F'72'73, "stallion" }
			});
		}

	public:
		static void AssertCanLoadTreeAroundLatestRootHash() {
			// Arrange:
			RunLoadTest([](auto& tree, auto&& checker) {
				// Act:
				EXPECT_TRUE(tree.tryLoad(checker.get("root")));

				// Assert:
				AssertTreeLoadRoot(tree, std::move(checker));
			});
		}

		static void AssertCanLoadTreeAroundPreviousRootHash() {
			// Arrange:
			RunLoadTest([](auto& tree, auto&& checker) {
				// Act:
				EXPECT_TRUE(tree.tryLoad(checker.get("root0")));

				// Assert:
				AssertTreeLoadRoot0(tree, std::move(checker));
			});
		}

		static void AssertCanLoadTreeAroundNonRootHash() {
			// Arrange:
			RunLoadTest([](auto& tree, auto&& checker) {
				// Act: this should be an error case, but it is expensive to check
				EXPECT_TRUE(tree.tryLoad(checker.get("puppy-coin")));

				// Assert:
				EXPECT_EQ(checker.get("puppy-coin"), tree.root());
				checker.checkReachable(tree.root(), { "puppy", "coin", "puppy-coin" });

				// - notice that no leaves are detected because the "root" is really a subtree, so key paths are not fully resolved
				AssertNotLeaves(tree, { { 0x64'6F'67'00 }, { 0x64'6F'67'65 } });
			});
		}

		static void AssertCanLoadTreeMultipleTimes() {
			// Arrange:
			RunLoadTest([](auto& tree, auto&& checker) {
				// Act:
				EXPECT_TRUE(tree.tryLoad(checker.get("puppy-coin")));
				EXPECT_TRUE(tree.tryLoad(checker.get("root0")));
				EXPECT_TRUE(tree.tryLoad(checker.get("root")));

				// Assert: only state of last load should be present
				AssertTreeLoadRoot(tree, std::move(checker));
			});
		}

		static void AssertCannotLoadTreeAroundUnknownHash() {
			// Arrange:
			RunLoadTest([](auto& tree, auto&& checker) {
				// Act:
				EXPECT_TRUE(tree.tryLoad(checker.get("root")));
				EXPECT_FALSE(tree.tryLoad(GenerateRandomByteArray<Hash256>()));

				// Assert: only state of last (successful) load should be present
				AssertTreeLoadRoot(tree, std::move(checker));
			});
		}

		// endregion

		// region setRoot

	public:
		static void AssertCanSetArbitraryRoot() {
			// Arrange:
			RunLoadTest([](auto& tree, auto&& checker) {
				auto randomHash = checker.addLeaf({ 0x20, 0x64, 0x6F, 0x67, 0x01 }, "random-node");
				auto randomNode = tree::LeafTreeNode(tree::TreeNodePath(0x64'6F'67'01), PassThroughEncoder::EncodeValue("random-node"));

				// Sanity:
				EXPECT_EQ(randomHash, randomNode.hash());

				// - cannot load the node because it is not in the data source
				EXPECT_TRUE(tree.tryLoad(checker.get("root")));
				EXPECT_FALSE(tree.tryLoad(randomHash));

				// Act: can forcibly set the root to a random node
				tree.setRoot(tree::TreeNode(randomNode));

				// Assert:
				EXPECT_EQ(checker.get("random-node"), tree.root());

				// - notice there is no reachable check because root is not in data source

				AssertLeaves(tree, { { 0x64'6F'67'01, "random-node" } });
			});
		}

		// endregion

		// region clear

	public:
		static void AssertCanClearTree() {
			// Arrange:
			RunLoadTest([](auto& tree, auto&& checker) {
				EXPECT_TRUE(tree.tryLoad(checker.get("root")));

				// Sanity:
				EXPECT_NE(Hash256(), tree.root());

				// Act:
				tree.clear();

				// Assert:
				EXPECT_EQ(Hash256(), tree.root());
			});
		}

		// endregion
	};

#define MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::PatriciaTreeTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_PATRICIA_TREE_TESTS(TRAITS_NAME) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, TreeHashIsInitiallyZero) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanInsertSingleValue) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoInsertSingleValue) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUpdateSingleValue) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanInsertSingleValueWithCustomEncoding) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoInsertSingleValueWithCustomEncoding) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanInsertMultipleValuesWithSamePrefix) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoInsertMultipleValuesWithSamePrefix) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanInsertMultipleValuesWithNoCommonPrefix) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoInsertMultipleValuesWithNoCommonPrefix) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanInsertNewValueIntoBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoInsertNewValueIntoBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUpdateValueInBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoSplitExtensionBranchNodeIntoBranchNodeWithLeafAndBranch) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanInsertNewValueIntoExtensionBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoInsertNewValueIntoExtensionBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUpdateValueInExtensionBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoSplitExtensionBranchNodeIntoExtensionBranchNodeWithLeafAndBranch) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanSplitLeafIntoBranchWithTwoLeaves) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoSplitLeafIntoBranchWithTwoLeaves) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanCreatePuppyTreeWithRootBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoCreatePuppyTreeWithRootBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanCreatePuppyTreeWithRootExtensionNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoCreatePuppyTreeWithRootExtensionNode) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, UnsetHasNoEffectWhenTreeIsEmpty) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, UnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootLeafNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, UnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, UnsetHasNoEffectWhenRemovingKeyNotInTreeWithRootExtensionNode) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, UnsetCanCreatePreviouslyUnseenRootLeaf) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, UnsetCanCreatePreviouslyUnseenRootBranch) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, UnsetCanCreatePreviouslyUnseenTree) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, LookupFailsWhenTreeIsEmpty) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, LookupFailsWhenKeyIsNotInTreeWithRootLeafNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, LookupFailsWhenKeyIsNotInTreeWithRootBranchNode) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, LookupFailsWhenKeyIsNotInTreeWithRootExtensionNode) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, LookupSucceedsWhenKeyIsTreeRoot) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, LookupSucceedsWhenKeyIsInTree) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanCreatePuppyTreeWithRootExtensionNode_AnyOrder) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanUndoPuppyTreeWithRootExtensionNode_AnyOrder) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanLoadTreeAroundLatestRootHash) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanLoadTreeAroundPreviousRootHash) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanLoadTreeAroundNonRootHash) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanLoadTreeMultipleTimes) \
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CannotLoadTreeAroundUnknownHash) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanSetArbitraryRoot) \
	\
	MAKE_PATRICIA_TREE_TEST(TRAITS_NAME, CanClearTree)
}}

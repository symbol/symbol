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

#include "catapult/crypto_voting/OtsTree.h"
#include "catapult/utils/RandomGenerator.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS OtsTreeTests

	namespace {
		constexpr auto OtsPrivateKey_Size = PrivateKey::Size;

		constexpr auto File_Header_Size = sizeof(OtsOptions) + sizeof(OtsKeyIdentifier);
		constexpr auto Level_Header_Size = OtsPublicKey::Size + sizeof(uint64_t) + sizeof(uint64_t);
		constexpr auto Level_Entry = OtsPrivateKey_Size + sizeof(OtsSignature);

		constexpr auto Start_Key = OtsKeyIdentifier{ 8, 4 };
		constexpr auto End_Key = OtsKeyIdentifier{ 13, 5 };
		constexpr auto Num_Batches = End_Key.BatchId - Start_Key.BatchId + 1;
		constexpr OtsOptions Default_Options{ 7, Start_Key, End_Key };

		constexpr auto L1_Size = Level_Header_Size + Num_Batches * Level_Entry;

		OtsKeyPairType GenerateKeyPair() {
			return test::GenerateKeyPair();
		}

		void AssertOptions(const OtsOptions& expected, const OtsOptions& actual) {
			EXPECT_EQ(expected.Dilution, actual.Dilution);
			EXPECT_EQ(expected.StartKeyIdentifier, actual.StartKeyIdentifier);
			EXPECT_EQ(expected.EndKeyIdentifier, actual.EndKeyIdentifier);
		}

		// region breadcrumb storage

		enum class OperationType {
			Seek,
			Write
		};

		struct Operation {
		public:
			static Operation Write(uint64_t value) {
				return { OperationType::Write, value };
			}

			static Operation Seek(uint64_t value) {
				return { OperationType::Seek, value };
			}

		public:
			constexpr bool operator==(const Operation& rhs) const {
				return Type == rhs.Type && Value == rhs.Value;
			}

		public:
			OperationType Type;
			uint64_t Value;
		};

		class BreadcrumbStorage : public io::SeekableStream {
		public:
			BreadcrumbStorage() : m_position(0)
			{}

		public:
			void write(const RawBuffer& buffer) override {
				Breadcrumbs.push_back(Operation::Write(buffer.Size));
				m_position += buffer.Size;
			}

			void flush() override
			{}

		public:
			bool eof() const override {
				CATAPULT_THROW_RUNTIME_ERROR("eof - not supported");
			}

			void read(const MutableRawBuffer&) override {
				CATAPULT_THROW_RUNTIME_ERROR("read - not supported");
			}

		public:
			void seek(uint64_t position) override {
				Breadcrumbs.push_back(Operation::Seek(position));
				m_position = position;
			}

			uint64_t position() const override {
				return m_position;
			}

		public:
			size_t m_position;
			std::vector<Operation> Breadcrumbs;
		};

		// endregion

		// region test context

		template<typename TStorage>
		class TestContext {
		public:
			TestContext()
					: m_tree(OtsTree::Create(GenerateKeyPair(), m_storage, Default_Options))
					, m_messageBuffer(test::GenerateRandomArray<10>())
			{}

			TestContext(TestContext& originalContext)
					: m_tree(OtsTree::FromStream(CopyInto(originalContext.m_storage, m_storage)))
					, m_messageBuffer(originalContext.m_messageBuffer)
			{}

		public:
			const auto& storage() const {
				return m_storage;
			}

			const auto& tree() const {
				return m_tree;
			}

			const auto& buffer() const {
				return m_messageBuffer;
			}

		public:
			auto sign(const OtsKeyIdentifier& keyIdentifier) {
				return m_tree.sign(keyIdentifier, m_messageBuffer);
			}

		private:
			static TStorage& CopyInto(const TStorage& source, TStorage& dest) {
				source.copyTo(dest);
				return dest;
			}

		private:
			TStorage m_storage;
			OtsTree m_tree;
			std::array<uint8_t, 10> m_messageBuffer;
		};

		using BreadcrumbTestContext = TestContext<BreadcrumbStorage>;
		using MockTestContext = TestContext<mocks::MockSeekableMemoryStream>;

		// endregion
	}

	// region can sign tests

	TEST(TEST_CLASS, CanSignReturnsFalseWhenKeyIdentifierIsBelowRange) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act + Assert:
		EXPECT_FALSE(context.tree().canSign({ Start_Key.BatchId - 1, Start_Key.KeyId }));
		EXPECT_FALSE(context.tree().canSign({ Start_Key.BatchId, Start_Key.KeyId - 1 }));
	}

	TEST(TEST_CLASS, CanSignReturnsFalseWhenKeyIdentifierIsAboveRange) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act + Assert:
		EXPECT_FALSE(context.tree().canSign({ End_Key.BatchId + 1, End_Key.KeyId }));
		EXPECT_FALSE(context.tree().canSign({ End_Key.BatchId, End_Key.KeyId + 1 }));
	}

	TEST(TEST_CLASS, CanSignReturnsTrueForValuesNearBoundaries) {
		// Arrange:
		BreadcrumbTestContext context;
		std::vector<OtsKeyIdentifier> keyIdentifiers{ Start_Key, End_Key };

		// Act + Assert:
		for (const auto& keyIdentifier : keyIdentifiers)
			EXPECT_TRUE(context.tree().canSign(keyIdentifier)) << keyIdentifier;
	}

	// endregion

	// region ctor

	TEST(TEST_CLASS, EmptyTreeHasProperRootPublicKeyAndOptions) {
		// Arrange:
		auto rootKeyPair = GenerateKeyPair();
		auto expectedPublicKey = rootKeyPair.publicKey();
		BreadcrumbStorage storage;

		// Act:
		auto tree = OtsTree::Create(std::move(rootKeyPair), storage, Default_Options);

		// Assert:
		EXPECT_EQ(expectedPublicKey, tree.rootPublicKey());
		AssertOptions(Default_Options, tree.options());
	}

	// endregion

	// region sign tests

	TEST(TEST_CLASS, SignForValuesNearBoundaries) {
		// Arrange:
		std::vector<OtsKeyIdentifier> keyIdentifiers{ Start_Key, End_Key };

		// Act: create new context for every run
		for (const auto& keyIdentifier : keyIdentifiers) {
			BreadcrumbTestContext context;
			auto signature = context.sign(keyIdentifier);

			// Assert:
			EXPECT_TRUE(Verify(signature, keyIdentifier, context.buffer()));
		}
	}

	TEST(TEST_CLASS, AccessingKeyIdentifierOutsideRangeThrows) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		EXPECT_NO_THROW(context.sign({ 10, Default_Options.Dilution - 1 }));
		EXPECT_THROW(context.sign({ 10, Default_Options.Dilution }), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AccessingKeyIdentifierBelowRangeThrows) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		EXPECT_THROW(context.sign({ Start_Key.BatchId - 1, Start_Key.KeyId }), catapult_runtime_error);
		EXPECT_THROW(context.sign({ Start_Key.BatchId, Start_Key.KeyId - 1 }), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AccessingKeyIdentifierAboveRangeThrows) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		EXPECT_THROW(context.sign({ End_Key.BatchId + 1, End_Key.KeyId }), catapult_runtime_error);
		EXPECT_THROW(context.sign({ End_Key.BatchId, End_Key.KeyId + 1 }), catapult_runtime_error);
	}

	TEST(TEST_CLASS, GeneratedSignaturesAreValid) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		auto signature1 = context.sign({ 8, 4 });
		auto signature2 = context.sign({ 8, 5 }); // KeyId incremenent
		auto signature3 = context.sign({ 9, 0 }); // BatchId increment
		auto signature4 = context.sign({ 11, 0 }); // BatchId skip
		auto signature5 = context.sign({ 13, 3 }); // BatchId and KeyId skip

		// Assert:
		EXPECT_TRUE(Verify(signature1, { 8, 4 }, context.buffer()));
		EXPECT_TRUE(Verify(signature2, { 8, 5 }, context.buffer()));
		EXPECT_TRUE(Verify(signature3, { 9, 0 }, context.buffer()));
		EXPECT_TRUE(Verify(signature4, { 11, 0 }, context.buffer()));
		EXPECT_TRUE(Verify(signature5, { 13, 3 }, context.buffer()));
	}

	// endregion

	namespace {
		// region storage checker

		class StorageChecker {
		private:
			static constexpr auto Num_Key_Identifier_Values = 2;
			static constexpr auto Num_Property_Values = 1 + 2 * Num_Key_Identifier_Values; // dilution, start, end

		public:
			explicit StorageChecker(const std::vector<Operation>& breadcrumbs)
					: m_operations(breadcrumbs)
					, m_index(0)
			{}

		public:
			void assertTreeHeader() {
				checkSize(Num_Key_Identifier_Values + Num_Property_Values);
				for (auto i = 0u; i < Num_Key_Identifier_Values + Num_Property_Values; ++i)
					EXPECT_EQ(Operation::Write(sizeof(uint64_t)), m_operations[m_index++]);
			}

			void assertSaveLevel(uint64_t position, size_t numEntries) {
				assertSeek(position);
				assertLevelHeader();
				assertLevelEntries(numEntries);
			}

			void assertWipePrivateKey(uint64_t levelOffset, uint64_t numLevelEntries, uint64_t entryId, size_t wipeEntries) {
				auto index = numLevelEntries - entryId - 1;
				assertWipes(levelOffset, index + 1, wipeEntries);
				assertWipes(levelOffset, index, 1);
			}

			void assertLastStep() {
				checkSize(1);
				EXPECT_EQ(Operation::Seek(sizeof(OtsOptions)), m_operations[m_index++]);

				checkSize(Num_Key_Identifier_Values);
				for (auto i = 0u; i < Num_Key_Identifier_Values; ++i)
					EXPECT_EQ(Operation::Write(sizeof(uint64_t)), m_operations[m_index++]);
			}

			void assertFinished() {
				EXPECT_EQ(m_operations.size(), m_index);
			}

		private:
			void checkSize(size_t numOperations) {
				if (m_operations.size() < m_index + numOperations)
					CATAPULT_THROW_RUNTIME_ERROR("too few operations captured");
			}

			void assertLevelHeader() {
				checkSize(3);
				EXPECT_EQ(Operation::Write(OtsPublicKey::Size), m_operations[m_index++]);
				EXPECT_EQ(Operation::Write(sizeof(uint64_t)), m_operations[m_index++]);
				EXPECT_EQ(Operation::Write(sizeof(uint64_t)), m_operations[m_index++]);
			}

			void assertLevelEntries(size_t numEntries) {
				checkSize(2 * numEntries);
				for (auto i = 0u; i < numEntries; ++i) {
					EXPECT_EQ(Operation::Write(OtsPrivateKey_Size), m_operations[m_index++]);
					EXPECT_EQ(Operation::Write(OtsSignature::Size), m_operations[m_index++]);
				}
			}

			void assertWipes(uint64_t levelOffset, uint64_t startIndex, size_t numEntries) {
				checkSize(2 * numEntries);

				auto fileOffset = File_Header_Size + Level_Header_Size + levelOffset;
				for (auto i = 0u; i < numEntries; ++i) {
					EXPECT_EQ(Operation::Seek(fileOffset + (startIndex + i) * Level_Entry), m_operations[m_index++]);
					EXPECT_EQ(Operation::Write(OtsPrivateKey_Size), m_operations[m_index++]);
				}
			}

			// all seeks are relative to File_Header_Size
			void assertSeek(uint64_t offset) {
				checkSize(1);
				EXPECT_EQ(Operation::Seek(File_Header_Size + offset), m_operations[m_index++]);
			}

		public:
			const std::vector<Operation>& m_operations;
			uint64_t m_index;
		};

		// endregion
	}

	// region storage - saving

	TEST(TEST_CLASS, NewlyCreatedTreeSavesHeaderAndFirstLevel) {
		// Act:
		BreadcrumbTestContext context;

		// Assert:
		StorageChecker checker(context.storage().Breadcrumbs);
		checker.assertTreeHeader();
		checker.assertSaveLevel(0, Num_Batches);
		checker.assertFinished();
	}

	namespace {
		void VerifyFull(StorageChecker& checker, const OtsKeyIdentifier& keyIdentifier) {
			// first level
			auto relativeBatchId = keyIdentifier.BatchId - Start_Key.BatchId;
			checker.assertTreeHeader();
			checker.assertSaveLevel(0, Num_Batches);
			checker.assertWipePrivateKey(0, Num_Batches, relativeBatchId, relativeBatchId);

			// second level
			// tree saves only starting at subId, so keys < subId aren't created/stored,
			// so only wiped key is the one that is == subId, that's why 0, 0 is passed below
			auto numEntriesLevel2 = Default_Options.Dilution - keyIdentifier.KeyId;
			checker.assertSaveLevel(L1_Size, numEntriesLevel2);
			checker.assertWipePrivateKey(L1_Size, numEntriesLevel2, 0, 0);

			checker.assertLastStep();
		}
	}

	TEST(TEST_CLASS, SignWipesProperKeysAndSavesSubLevels) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		context.sign({ 9, 2 });

		// Assert:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 9, 2 });
		checker.assertFinished();
	}

	TEST(TEST_CLASS, AccessingSubsequentKeyDoesSingleAdditionalWipe) {
		// Arrange:
		BreadcrumbTestContext context;
		context.sign({ 9, 2 });

		// Sanity:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 9, 2 });

		// Act:
		context.sign({ 9, 3 });

		// Assert:
		// - keys (9, 0), (9, 1) weren't saved
		// - keys (9, 2), (9, 3) have been wiped
		// - due to the way tracking works, key (9, 2) will be wiped again
		checker.assertWipePrivateKey(L1_Size, Default_Options.Dilution - 2, 1, 1);
		checker.assertLastStep();

		checker.assertFinished();
	}

	TEST(TEST_CLASS, AccessingFurtherKeyWipesProperKeys) {
		// Arrange:
		BreadcrumbTestContext context;
		context.sign({ 9, 2 });

		// Sanity:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 9, 2 });

		// Act:
		context.sign({ 9, 5 });

		// Assert:
		// - keys (9, 0), (9, 1) weren't saved
		// - keys (9, 2), (9, 3), (9, 4), (9, 5) have been wiped
		// - due to the way tracking works, key (9, 2) will be wiped again
		checker.assertWipePrivateKey(L1_Size, Default_Options.Dilution - 2, 3, 3);
		checker.assertLastStep();

		checker.assertFinished();
	}

	TEST(TEST_CLASS, AccessingDifferentBatchIdGeneratesNewSubKeys) {
		// Arrange:
		BreadcrumbTestContext context;
		context.sign({ 9, 2 });

		// Sanity:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 9, 2 });

		// Act:
		context.sign({ 13, 2 });

		// Assert:
		// - top level keys will be wiped (9,) (10,) (11,) (12,)
		// - (13,) has index 5, in top level there are keys (8, 9, ... 12, 13)
		checker.assertWipePrivateKey(0, Num_Batches, 5, 4);

		// - 13 is last batch id, it should contain only keys 2-5, (0 and 1 will be skipped)
		constexpr auto Num_Lower_Level_Keys = 4;
		checker.assertSaveLevel(L1_Size, Num_Lower_Level_Keys);

		// - get the key at (13, 2)
		checker.assertWipePrivateKey(L1_Size, Num_Lower_Level_Keys, 0, 0);
		checker.assertLastStep();

		checker.assertFinished();
	}

	// endregion

	// region roundtrip

	namespace {
		constexpr bool SingleLevelMatch(const OtsTreeSignature& lhs, const OtsTreeSignature& rhs) {
			return lhs.Root.ParentPublicKey == rhs.Root.ParentPublicKey
					&& lhs.Root.Signature == rhs.Root.Signature
					&& lhs.Top.ParentPublicKey == rhs.Top.ParentPublicKey;
		}
	}

	TEST(TEST_CLASS, RoundtripTestSingleLevelTree) {
		// Arrange:
		MockTestContext originalContext;

		// Act: reload single level tree from storage
		MockTestContext context(originalContext);

		// Assert: signatures are different, but all top level keys should match
		// '4' is used as KeyId, as it will be present in every BatchId
		EXPECT_EQ(originalContext.tree().rootPublicKey(), context.tree().rootPublicKey());
		AssertOptions(originalContext.tree().options(), context.tree().options());

		for (auto batchId = Start_Key.BatchId; batchId <= End_Key.BatchId; ++batchId) {
			auto originalSignature = originalContext.sign({ batchId, 4 });
			auto signature = context.sign({ batchId, 4 });
			EXPECT_TRUE(SingleLevelMatch(originalSignature, signature));
			EXPECT_NE(originalSignature, signature);
		}
	}

	TEST(TEST_CLASS, RoundtripTestAccessingUsedKeyThrows) {
		// Arrange:
		MockTestContext originalContext;
		OtsKeyIdentifier usedIdentifier{ 9, 2 };
		OtsKeyIdentifier unusedIdentifier{ 9, 3 };
		originalContext.sign(usedIdentifier);

		// Act: reload tree from storage
		MockTestContext context(originalContext);

		// Assert:
		EXPECT_FALSE(originalContext.tree().canSign(usedIdentifier));
		EXPECT_FALSE(context.tree().canSign(usedIdentifier));
		EXPECT_TRUE(originalContext.tree().canSign(unusedIdentifier));
		EXPECT_TRUE(context.tree().canSign(unusedIdentifier));

		EXPECT_THROW(originalContext.sign(usedIdentifier), catapult_runtime_error);
		EXPECT_THROW(context.sign(usedIdentifier), catapult_runtime_error);
	}

	TEST(TEST_CLASS, RoundtripTestMultiLevelTree) {
		// Arrange:
		MockTestContext originalContext;
		originalContext.sign({ 9, 2 });

		// Act: reload tree from storage
		MockTestContext context(originalContext);

		// Assert:
		// - signatures on bottom level from both trees should match
		// - all signatures using keys (9, 3) - (9, 6) should match
		for (auto keyId = 3u; keyId < Default_Options.Dilution; ++keyId) {
			auto expectedSignature = originalContext.sign({ 9, keyId });
			auto signature = context.sign({ 9, keyId });
			EXPECT_EQ(expectedSignature, signature);
		}

		// - signature at different batch id will match only partially
		auto expectedSignature = originalContext.sign({ 10, 0 });
		auto signature = context.sign({ 10, 0 });
		EXPECT_NE(expectedSignature, signature);
		EXPECT_TRUE(SingleLevelMatch(expectedSignature, signature));
	}

	// endregion
}}

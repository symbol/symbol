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
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS OtsTreeTests

	namespace {
		constexpr auto OtsPrivateKey_Size = PrivateKey::Size;

		constexpr auto File_Header_Size = sizeof(OtsOptions) + sizeof(StepIdentifier);
		constexpr auto Level_Header_Size = OtsPublicKey::Size + sizeof(uint64_t) + sizeof(uint64_t);
		constexpr auto Level_Entry = OtsPrivateKey_Size + sizeof(OtsSignature);

		// finalization points used 30-40 - inclusive on both ends so 11 keys
		constexpr auto Start_Point = 30u;
		constexpr auto End_Point = 40u;
		constexpr auto Num_Finalization_Points = End_Point - Start_Point + 1;
		constexpr OtsOptions Default_Options{ 10, 8 };

		constexpr auto L1_Size = Level_Header_Size + Num_Finalization_Points * Level_Entry;
		constexpr auto L2_Size = Level_Header_Size + Default_Options.MaxRounds * Level_Entry;

		OtsKeyPairType GenerateKeyPair() {
			return test::GenerateKeyPair();
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

		class BreadcrumbStorage : public SeekableOutputStream {
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

		// region mock storage

		class MockStorage : public SeekableOutputStream, public io::InputStream {
		public:
			MockStorage() : m_position(0)
			{}

		public:
			void write(const RawBuffer& buffer) override {
				m_buffer.resize(std::max<size_t>(m_buffer.size(), m_position + buffer.Size));
				std::memcpy(&m_buffer[m_position], buffer.pData, buffer.Size);
				m_position += buffer.Size;
			}

			void flush() override
			{}

			void seek(uint64_t position) override {
				m_position = position;
			}

			uint64_t position() const override {
				return m_position;
			}

			bool eof() const override {
				return false;
			}

			void read(const MutableRawBuffer& buffer) override {
				if (m_position + buffer.Size > m_buffer.size())
					CATAPULT_THROW_RUNTIME_ERROR("invalid read()");

				std::memcpy(buffer.pData, &m_buffer[m_position], buffer.Size);
				m_position += buffer.Size;
			}

		private:
			std::vector<uint8_t> m_buffer;
			uint64_t m_position;
		};

		// endregion

		// region test context

		template<typename TStorage>
		class TestContext {
		public:
			TestContext()
					: m_tree(OtsTree::Create(
							GenerateKeyPair(),
							m_storage,
							FinalizationPoint(Start_Point),
							FinalizationPoint(End_Point),
							Default_Options))
					, m_messageBuffer(test::GenerateRandomArray<10>())
			{}

			TestContext(TestContext& originalContext)
					: m_tree(OtsTree::FromStream(originalContext.m_storage, m_storage))
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
			auto seekToBegin() {
				m_storage.seek(0);
			}

			auto sign(const StepIdentifier& stepIdentifier) {
				return m_tree.sign(stepIdentifier, m_messageBuffer);
			}

		private:
			TStorage m_storage;
			OtsTree m_tree;
			std::array<uint8_t, 10> m_messageBuffer;
		};

		using BreadcrumbTestContext = TestContext<BreadcrumbStorage>;
		using MockTestContext = TestContext<MockStorage>;

		// endregion
	}

	// region can sign tests

	TEST(TEST_CLASS, CanSignReturnsFalseWhenFinalizationPointIsBelowRange) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act + Assert:
		EXPECT_FALSE(context.tree().canSign({ 29, 0, 0 }));
	}

	TEST(TEST_CLASS, CanSignReturnsFalseWhenFinalizationPointIsAboveRange) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act + Assert:
		EXPECT_FALSE(context.tree().canSign({ 41, 0, 0 }));
	}

	TEST(TEST_CLASS, CanSignReturnsFalseWhenRoundIsOutsideRange) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act + Assert:
		EXPECT_FALSE(context.tree().canSign({ 35, Default_Options.MaxRounds, 0 }));
	}

	TEST(TEST_CLASS, CanSignReturnsFalseWhenSubRoundIsOutsideRange) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act + Assert:
		EXPECT_FALSE(context.tree().canSign({ 35, 0, Default_Options.MaxSubRounds }));
	}

	TEST(TEST_CLASS, CanSignReturnsTrueForValuesNearBoundaries) {
		// Arrange:
		BreadcrumbTestContext context;
		std::vector<StepIdentifier> stepIdentifiers{
			{ Start_Point, 0, 0 },
			{ End_Point, 0, 0 },
			{ End_Point, Default_Options.MaxRounds - 1, 0 },
			{ End_Point, Default_Options.MaxRounds - 1, Default_Options.MaxSubRounds - 1 }
		};

		// Act + Assert:
		for (const auto& stepIdentifier : stepIdentifiers)
			EXPECT_TRUE(context.tree().canSign(stepIdentifier)) << stepIdentifier;
	}

	// endregion

	// region ctor

	TEST(TEST_CLASS, EmptyTreeHasProperRootPublicKey) {
		// Arrange:
		auto rootKeyPair = GenerateKeyPair();
		auto expectedPublicKey = rootKeyPair.publicKey();
		BreadcrumbStorage storage;

		// Act:
		auto tree = OtsTree::Create(
				std::move(rootKeyPair),
				storage,
				FinalizationPoint(Start_Point),
				FinalizationPoint(End_Point),
				Default_Options);

		// Assert:
		EXPECT_EQ(expectedPublicKey, tree.rootPublicKey());
	}

	// endregion

	// region sign tests

	TEST(TEST_CLASS, SignForValuesNearBoundaries) {
		// Arrange:
		std::vector<StepIdentifier> stepIdentifiers{
			{ Start_Point, 0, 0 },
			{ End_Point, 0, 0 },
			{ End_Point, Default_Options.MaxRounds - 1, 0 },
			{ End_Point, Default_Options.MaxRounds - 1, Default_Options.MaxSubRounds - 1 }
		};

		// Act: create new context for every run
		for (const auto& stepIdentifier : stepIdentifiers) {
			BreadcrumbTestContext context;
			auto signature = context.sign(stepIdentifier);

			// Assert:
			EXPECT_TRUE(Verify(signature, stepIdentifier, context.buffer()));
		}
	}

	TEST(TEST_CLASS, AccessingSubroundOutsideRangeThrows) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		EXPECT_NO_THROW(context.sign({ 32, 2, Default_Options.MaxSubRounds - 1 }));
		EXPECT_THROW(context.sign({ 32, 2, Default_Options.MaxSubRounds }), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AccessingRoundOutsideRangeThrows) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		EXPECT_NO_THROW(context.sign({ 32, Default_Options.MaxRounds - 1, 4 }));
		EXPECT_THROW(context.sign({ 32, Default_Options.MaxRounds, 4 }), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AccessingFinalizationPointBelowRangeThrows) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		EXPECT_THROW(context.sign({ 29, 2, 4 }), catapult_runtime_error);
	}

	TEST(TEST_CLASS, AccessingFinalizationPointAboveRangeThrows) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		EXPECT_THROW(context.sign({ 41, 0, 0 }), catapult_runtime_error);
	}

	TEST(TEST_CLASS, GeneratedSignaturesAreValid) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		auto signature1 = context.sign({ 32, 2, 4 });
		auto signature2 = context.sign({ 32, 3, 4 });
		auto signature3 = context.sign({ 32, 3, 6 });
		auto signature4 = context.sign({ 32, 3, 7 });
		auto signature5 = context.sign({ 35, 0, 0 });
		auto signature6 = context.sign({ 35, 0, 7 });

		// Assert:
		EXPECT_TRUE(Verify(signature1, { 32, 2, 4 }, context.buffer()));
		EXPECT_TRUE(Verify(signature2, { 32, 3, 4 }, context.buffer()));
		EXPECT_TRUE(Verify(signature3, { 32, 3, 6 }, context.buffer()));
		EXPECT_TRUE(Verify(signature4, { 32, 3, 7 }, context.buffer()));
		EXPECT_TRUE(Verify(signature5, { 35, 0, 0 }, context.buffer()));
		EXPECT_TRUE(Verify(signature6, { 35, 0, 7 }, context.buffer()));
	}

	// endregion

	namespace {
		// region storage checker

		class StorageChecker {
		private:
			static constexpr auto Num_Step_Identifier_Values = 3;
			static constexpr auto Num_Property_Values = 2;

		public:
			explicit StorageChecker(const std::vector<Operation>& breadcrumbs)
					: m_operations(breadcrumbs)
					, m_index(0)
			{}

		public:
			void assertTreeHeader() {
				checkSize(Num_Step_Identifier_Values + Num_Property_Values);
				for (auto i = 0u; i < Num_Step_Identifier_Values + Num_Property_Values; ++i)
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

				checkSize(Num_Step_Identifier_Values);
				for (auto i = 0u; i < Num_Step_Identifier_Values; ++i)
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
		checker.assertSaveLevel(0, Num_Finalization_Points);
		checker.assertFinished();
	}

	namespace {
		void VerifyFull(StorageChecker& checker, const StepIdentifier& stepIdentifier) {
			// first level
			auto pointId = stepIdentifier.Point - Start_Point;
			checker.assertTreeHeader();
			checker.assertSaveLevel(0, Num_Finalization_Points);
			checker.assertWipePrivateKey(0, Num_Finalization_Points, pointId, pointId);

			// second level
			checker.assertSaveLevel(L1_Size, Default_Options.MaxRounds);
			checker.assertWipePrivateKey(L1_Size, Default_Options.MaxRounds, stepIdentifier.Round, stepIdentifier.Round);

			// third level
			checker.assertSaveLevel(L1_Size + L2_Size, Default_Options.MaxSubRounds);
			checker.assertWipePrivateKey(
					L1_Size + L2_Size,
					Default_Options.MaxSubRounds,
					stepIdentifier.SubRound,
					stepIdentifier.SubRound);
			checker.assertLastStep();
		}
	}

	TEST(TEST_CLASS, SignWipesProperKeysAndSavesSubLevels) {
		// Arrange:
		BreadcrumbTestContext context;

		// Act:
		context.sign({ 32, 7, 3 });

		// Assert:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 32, 7, 3});
		checker.assertFinished();
	}

	TEST(TEST_CLASS, AccessingSubsequentKeyDoesSingleAdditionalWipe) {
		// Arrange:
		BreadcrumbTestContext context;
		context.sign({ 32, 7, 3 });

		// Sanity:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 32, 7, 3});

		// Act:
		context.sign({ 32, 7, 4 });

		// Arrange: get key at 4, keys 0,1,2,3 has been wiped, due to the way tracking works, key 3 will be wiped again
		checker.assertWipePrivateKey(L1_Size + L2_Size, Default_Options.MaxSubRounds, 4, 1);
		checker.assertLastStep();

		checker.assertFinished();
	}

	TEST(TEST_CLASS, AccessingFurtherKeyWipesProperKeys) {
		// Arrange:
		BreadcrumbTestContext context;
		context.sign({ 32, 7, 3 });

		// Sanity:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 32, 7, 3});

		// Act:
		context.sign({ 32, 7, 5 });

		// Arrange: get key at 5, additional keys that will be wiped are 3,4
		checker.assertWipePrivateKey(L1_Size + L2_Size, Default_Options.MaxSubRounds, 5, 2);
		checker.assertLastStep();

		checker.assertFinished();
	}

	TEST(TEST_CLASS, AccessingDifferentRoundKeyGeneratesNewSubroundKeys) {
		// Arrange:
		BreadcrumbTestContext context;
		context.sign({ 32, 2, 4 });

		// Sanity:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 32, 2, 4 });

		// Act:
		context.sign({ 32, 3, 4 });

		// Arrange:
		// - get key {32,3} build subround, due to the way tracking works, key {32,2} will be wiped again
		checker.assertWipePrivateKey(L1_Size, Default_Options.MaxRounds, 3, 1);
		checker.assertSaveLevel(L1_Size + L2_Size, Default_Options.MaxSubRounds);

		// - get the key at {32,3,4}
		checker.assertWipePrivateKey(L1_Size + L2_Size, Default_Options.MaxSubRounds, 4, 4);
		checker.assertLastStep();

		checker.assertFinished();
	}

	TEST(TEST_CLASS, AccessingDifferentFinalizationPointsKeyGeneratesNewRoundAndSubroundKeys) {
		// Arrange:
		BreadcrumbTestContext context;
		context.sign({ 32, 2, 4 });

		// Sanity:
		StorageChecker checker(context.storage().Breadcrumbs);
		VerifyFull(checker, { 32, 2, 4 });

		// Act:
		context.sign({ 35, 0, 0 });

		// Arrange:
		// - get key 35, build round keys, due to the way tracking works, key 32 will be wiped again, so total = 3
		checker.assertWipePrivateKey(0, Num_Finalization_Points, 5, 3);
		checker.assertSaveLevel(L1_Size, Default_Options.MaxRounds);

		// - get the key at {35,0}
		checker.assertWipePrivateKey(L1_Size, Default_Options.MaxRounds, 0, 0);
		checker.assertSaveLevel(L1_Size + L2_Size, Default_Options.MaxSubRounds);

		// - get the key at {35,0,0}
		checker.assertWipePrivateKey(L1_Size + L2_Size, Default_Options.MaxSubRounds, 0, 0);
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

		constexpr bool TwoLevelMatch(const OtsTreeSignature& lhs, const OtsTreeSignature& rhs) {
			return SingleLevelMatch(lhs, rhs)
					&& lhs.Top.Signature == rhs.Top.Signature
					&& lhs.Middle.ParentPublicKey == rhs.Middle.ParentPublicKey;
		}
	}

	TEST(TEST_CLASS, RoundtripTestSingleLevelTree) {
		// Arrange:
		MockTestContext originalContext;
		originalContext.seekToBegin();

		// Act: reload single level tree from storage
		MockTestContext context(originalContext);

		// Assert: signatures are different, but all top level keys should match
		EXPECT_EQ(originalContext.tree().rootPublicKey(), context.tree().rootPublicKey());
		for (auto point = Start_Point; point <= End_Point; ++point) {
			auto originalSignature = originalContext.sign({ point, 0, 0 });
			auto signature = context.sign({ point, 0, 0 });
			EXPECT_TRUE(SingleLevelMatch(originalSignature, signature));
			EXPECT_FALSE(TwoLevelMatch(originalSignature, signature));
		}
	}

	TEST(TEST_CLASS, RoundtripTestAccessingUsedKeyThrows) {
		// Arrange:
		MockTestContext originalContext;
		StepIdentifier usedIdentifier{ 32, 4, 2 };
		StepIdentifier unusedIdentifier{ 32, 4, 3 };
		originalContext.sign(usedIdentifier);
		originalContext.seekToBegin();

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
		originalContext.sign({ 32, 4, 2 });
		originalContext.seekToBegin();

		// Act: reload tree from storage
		MockTestContext context(originalContext);

		// Assert: signatures on bottom level from both trees should match
		for (auto subRound = 3u; subRound < Default_Options.MaxSubRounds; ++subRound) {
			auto expectedSignature = originalContext.sign({ 32, 4, subRound });
			auto signature = context.sign({ 32, 4, subRound });
			EXPECT_EQ(expectedSignature, signature);
		}

		// - signature at different round will match only partially.
		auto expectedSignature = originalContext.sign({ 32, 5, 0 });
		auto signature = context.sign({ 32, 5, 0 });
		EXPECT_NE(expectedSignature, signature);
		EXPECT_TRUE(TwoLevelMatch(expectedSignature, signature));
	}

	// endregion
}}

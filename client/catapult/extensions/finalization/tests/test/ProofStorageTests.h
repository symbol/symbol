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
#include "finalization/src/io/ProofStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace test {

	/// Proof storage test suite.
	template<typename TTraits>
	struct ProofStorageTests {
	private:
		// region storage context

		enum class PreparationMode { None, Default };

		class StorageContext {
		public:
			explicit StorageContext(PreparationMode mode)
					: m_pTempDirectoryGuard(std::make_unique<TempDirectoryGuard>())
					, m_pStorage(PrepareProofStorage(m_pTempDirectoryGuard->name(), mode))
			{}

		public:
			io::ProofStorage& operator*() {
				return *m_pStorage;
			}

			io::ProofStorage* operator->() {
				return m_pStorage.get();
			}

		private:
			static std::unique_ptr<io::ProofStorage> PrepareProofStorage(const std::string& destination, PreparationMode mode) {
				if (PreparationMode::Default == mode) {
					const std::string nemesisDirectory = "/00000";
					boost::filesystem::create_directories(destination + nemesisDirectory);

					FakeFinalizationPoint(destination, 2);
					SetIndexFinalizationPoint(destination, 1);
				}

				return TTraits::CreateStorage(destination);
			}

			static void SetIndexFinalizationPoint(const std::string& destination, uint64_t finalizationPoint) {
				io::RawFile indexFile(destination + "/proof.index.dat", io::OpenMode::Read_Write);
				io::Write64(indexFile, finalizationPoint);
			}

			static void FakeFinalizationPoint(const std::string& destination, uint64_t numFinalizationPoints) {
				const std::string nemesisDirectory = "/00000";
				const std::string nemesisHashFilename = destination + nemesisDirectory + "/proof.hashes.dat";

				std::vector<uint8_t> hashesBuffer(numFinalizationPoints * model::HeightHashPair::Size);
				{
					io::RawFile file(nemesisHashFilename, io::OpenMode::Read_Write);
					file.write(hashesBuffer);
				}
			}

		private:
			std::unique_ptr<TempDirectoryGuard> m_pTempDirectoryGuard;
			std::unique_ptr<io::ProofStorage> m_pStorage;
		};

		// endregion

		// region test utils

		static auto GenerateProof(size_t numVotes, FinalizationPoint finalizationPoint) {
			io::FinalizationProof proof;

			auto hash = GenerateRandomByteArray<Hash256>();
			model::StepIdentifier stepIdentifier = { finalizationPoint.unwrap(), 123 };
			for (auto i = 0u; i < numVotes; ++i)
				proof.push_back(CreateMessage(stepIdentifier, hash));

			return proof;
		}

		static auto PrepareStorageWithProofs(size_t numProofs) {
			StorageContext context(PreparationMode::Default);
			for (auto i = 2u; i <= numProofs; ++i) {
				auto newProof = GenerateProof(5, FinalizationPoint(i));
				context->saveProof(Height(100 + 2 * i), newProof);
			}

			return context;
		}

		// endregion

		// region assert helpers

		static void AssertStorageIndexes(const io::ProofStorage& storage, FinalizationPoint finalizationPoint, Height height) {
			EXPECT_EQ(finalizationPoint, storage.finalizationPoint());
			EXPECT_EQ(height, storage.finalizedHeight());
		}

		static void AssertVoteProof(const model::FinalizationMessage& expectedMessage, const model::VoteProof& voteProof) {
			EXPECT_EQ(expectedMessage.Signature, voteProof.Signature);
		}

		static void AssertSerializedProof(
				const io::FinalizationProof& expectedProof,
				Height expectedHeight,
				const model::PackedFinalizationProof& packedProof) {
			ASSERT_EQ(expectedProof.size(), packedProof.VoteProofsCount);

			const auto& firstMessage = *expectedProof[0];
			EXPECT_EQ(*firstMessage.HashesPtr(), packedProof.FinalizedHash);
			EXPECT_EQ(expectedHeight, packedProof.FinalizedHeight);
			EXPECT_EQ(firstMessage.StepIdentifier, packedProof.StepIdentifier);

			const auto* pVoteProof = packedProof.VoteProofsPtr();
			for (const auto& pMessage : expectedProof)
				AssertVoteProof(*pMessage, *pVoteProof++);
		}

		static void AssertHashes(
				const model::HeightHashPairRange& heightHashPairRange,
				std::initializer_list<model::HeightHashPair> expectedHeightHashPairs) {
			ASSERT_EQ(expectedHeightHashPairs.size(), heightHashPairRange.size());

			auto iter = heightHashPairRange.cbegin();
			for (const auto& expectedPair : expectedHeightHashPairs) {
				EXPECT_EQ(expectedPair.Height, iter->Height);
				EXPECT_EQ(expectedPair.Hash, iter->Hash);
				++iter;
			}
		}

		// endregion

	public:
		// region finalizationPoint + finalizedHeight

		static void AssertFinalizatationPointReturnsZeroWhenIndexDoesNotExist() {
			// Arrange:
			StorageContext pStorage(PreparationMode::None);

			// Act:
			auto finalizationPoint = pStorage->finalizationPoint();

			// Assert:
			EXPECT_EQ(FinalizationPoint(), finalizationPoint);
		}

		static void AssertFinalizedHeightReturnsZeroWhenIndexDoesNotExist() {
			// Arrange:
			StorageContext pStorage(PreparationMode::None);

			// Act:
			auto height = pStorage->finalizedHeight();

			// Assert:
			EXPECT_EQ(Height(), height);
		}

		// endregion

		// region saveProof - success

		static void AssertSavingProofWithFinalizationPointHigherThanCurrentFinalizationPointAltersFinalizationIndexes() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			auto proof1 = GenerateProof(3, FinalizationPoint(11));
			auto proof2 = GenerateProof(3, FinalizationPoint(12));
			pStorage->saveProof(Height(123), proof1);

			// Sanity:
			AssertStorageIndexes(*pStorage, FinalizationPoint(11), Height(123));

			// Act:
			pStorage->saveProof(Height(125), proof2);

			// Assert:
			AssertStorageIndexes(*pStorage, FinalizationPoint(12), Height(125));
		}

		static void AssertCanLoadNewlySavedProof() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			auto newProof = GenerateProof(3, FinalizationPoint(11));

			// Act:
			pStorage->saveProof(Height(123), newProof);

			auto pProof = pStorage->loadProof(FinalizationPoint(11));
			auto hashes = pStorage->loadFinalizedHashesFrom(FinalizationPoint(11), 100);

			// Assert:
			AssertStorageIndexes(*pStorage, FinalizationPoint(11), Height(123));
			AssertSerializedProof(newProof, Height(123), *pProof);
			AssertHashes(hashes, { { Height(123), *newProof.front()->HashesPtr() } });
		}

		static void AssertSaveProofUsesDataFromFirstMessage() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			auto hash1 = GenerateRandomByteArray<Hash256>();
			auto hash2 = GenerateRandomByteArray<Hash256>();

			io::FinalizationProof proof;
			proof.push_back(CreateMessage({ 11, 78 }, hash1));
			proof.push_back(CreateMessage({ 42, 90 }, hash2));

			// Act:
			pStorage->saveProof(Height(123), proof);

			auto pProof = pStorage->loadProof(FinalizationPoint(11));

			// Assert:
			AssertStorageIndexes(*pStorage, FinalizationPoint(11), Height(123));
			EXPECT_EQ(hash1, pProof->FinalizedHash);
			EXPECT_EQ(Height(123), pProof->FinalizedHeight);
			EXPECT_EQ(model::StepIdentifier({ 11, 78 }), pProof->StepIdentifier);

			const auto* pVoteProof = pProof->VoteProofsPtr();
			for (const auto& pMessage : proof)
				AssertVoteProof(*pMessage, *pVoteProof++);

			// - finalization point from second message is not accessible
			EXPECT_THROW(pStorage->loadProof(FinalizationPoint(42)), catapult_invalid_argument);
		}

		// endregion

		// region saveProof - failure

	private:
		static void AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint newFinalizationPoint) {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			auto newProof = GenerateProof(3, newFinalizationPoint);

			// Act + Assert:
			EXPECT_THROW(pStorage->saveProof(Height(123), newProof), catapult_invalid_argument);
		}

		static void AssertCannotSaveProofAtHeight(Height newFinalizedHeight) {
			// Arrange: prepare storage with proofs for heights 104-120
			auto pStorage = PrepareStorageWithProofs(10);
			auto newProof = GenerateProof(3, FinalizationPoint(11));

			// Act + Assert:
			EXPECT_THROW(pStorage->saveProof(newFinalizedHeight, newProof), catapult_invalid_argument);
		}

		static void AssertCanSaveProofAtHeight(Height newFinalizedHeight) {
			// Arrange: prepare storage with proofs for heights 104-120
			auto pStorage = PrepareStorageWithProofs(10);
			auto newProof = GenerateProof(3, FinalizationPoint(11));

			// Act + Assert:
			EXPECT_NO_THROW(pStorage->saveProof(newFinalizedHeight, newProof));
		}

	public:
		static void AssertCannotSaveEmptyProof() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			io::FinalizationProof proof;

			// Act + Assert:
			EXPECT_THROW(pStorage->saveProof(Height(123), proof), catapult_invalid_argument);
		}

		static void AssertCannotSaveProofWithFinalizationPointLessThanCurrentFinalizationPoint() {
			AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(1));
			AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(9));
		}

		static void AssertCannotSaveProofMoreThanOneFinalizationPointBeyondCurrentFinalizationPoint() {
			AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(12));
			AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(110));
		}

		static void AssertCannotSaveProofWithHeightLessOrEqualToCurrentHeight() {
			AssertCannotSaveProofAtHeight(Height(109));
			AssertCannotSaveProofAtHeight(Height(120));
		}

		static void AssertCanSaveProofWithHeightGreaterThanCurrentHeight() {
			AssertCanSaveProofAtHeight(Height(121));
			AssertCanSaveProofAtHeight(Height(125));
			AssertCanSaveProofAtHeight(Height(150));
		}

		// endregion

		// region loadFinalizedHashesFrom

		static void AssertCanLoadHashesAtFinalizationPointGreaterThanCurrentFinalizationPoint() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			// Act:
			auto hashes = pStorage->loadFinalizedHashesFrom(FinalizationPoint(11), 100);

			// Assert:
			AssertHashes(hashes, {});
		}

		static void AssertLoadingHashesAtFinalizationPointZeroReturnsEmptyRange() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			// Act:
			auto hashes = pStorage->loadFinalizedHashesFrom(FinalizationPoint(0), 100);

			// Assert:
			AssertHashes(hashes, {});
		}

		// endregion

		// region loadProof

		static void AssertCanLoadProofAtFinalizationPointLessThanCurrentFinalizationPoint() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto proof1 = GenerateProof(3, FinalizationPoint(11));
			pStorage->saveProof(Height(123), proof1);

			auto proof2 = GenerateProof(3, FinalizationPoint(12));
			pStorage->saveProof(Height(125), proof2);

			// Act:
			auto pProof = pStorage->loadProof(FinalizationPoint(11));

			// Assert:
			AssertStorageIndexes(*pStorage, FinalizationPoint(12), Height(125));
			AssertSerializedProof(proof1, Height(123), *pProof);
		}

		static void AssertCannotLoadProofAtFinalizationPointGreaterThanCurrentFinalizationPoint() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			// Act + Assert:
			EXPECT_THROW(pStorage->loadProof(FinalizationPoint(11)), catapult_invalid_argument);
		}

		static void AssertCanLoadMultipleSavedProofs() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto proof1 = GenerateProof(3, FinalizationPoint(11));
			auto proof2 = GenerateProof(3, FinalizationPoint(12));
			pStorage->saveProof(Height(123), proof1);
			pStorage->saveProof(Height(125), proof2);

			// Act:
			auto pLoadedProof1 = pStorage->loadProof(FinalizationPoint(11));
			auto pLoadedProof2 = pStorage->loadProof(FinalizationPoint(12));
			auto hashes = pStorage->loadFinalizedHashesFrom(FinalizationPoint(11), 100);

			// Assert:
			AssertStorageIndexes(*pStorage, FinalizationPoint(12), Height(125));
			AssertSerializedProof(proof1, Height(123), *pLoadedProof1);
			AssertSerializedProof(proof2, Height(125), *pLoadedProof2);
			AssertHashes(hashes, {
				{ Height(123), *proof1.front()->HashesPtr() },
				{ Height(125), *proof2.front()->HashesPtr() }
			});
		}

		// endregion
	};
}}

// region MAKE/DEFINE TESTs

#define MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::ProofStorageTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_PROOF_STORAGE_TESTS(TRAITS_NAME) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, FinalizatationPointReturnsZeroWhenIndexDoesNotExist) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, FinalizedHeightReturnsZeroWhenIndexDoesNotExist) \
	\
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, SavingProofWithFinalizationPointHigherThanCurrentFinalizationPointAltersFinalizationIndexes) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadNewlySavedProof) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, SaveProofUsesDataFromFirstMessage) \
	\
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotSaveEmptyProof) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotSaveProofWithFinalizationPointLessThanCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotSaveProofMoreThanOneFinalizationPointBeyondCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotSaveProofWithHeightLessOrEqualToCurrentHeight) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanSaveProofWithHeightGreaterThanCurrentHeight) \
	\
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadHashesAtFinalizationPointGreaterThanCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, LoadingHashesAtFinalizationPointZeroReturnsEmptyRange) \
	\
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadProofAtFinalizationPointLessThanCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotLoadProofAtFinalizationPointGreaterThanCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadMultipleSavedProofs)

// endregion

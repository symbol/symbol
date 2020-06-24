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

#include "finalization/src/io/FileProofStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace io {

#define TEST_CLASS FileProofStorageTests

	namespace {
		// region storage context

		class StorageContext {
		public:
			StorageContext()
					: m_pTempDirectoryGuard(std::make_unique<test::TempDirectoryGuard>())
					, m_pStorage(PrepareProofStorage(m_pTempDirectoryGuard->name()))
			{}

		public:
			FileProofStorage& operator*() {
				return *m_pStorage;
			}

			FileProofStorage* operator->() {
				return m_pStorage.get();
			}

		private:
			static std::unique_ptr<FileProofStorage> PrepareProofStorage(const std::string& destination) {
				const std::string nemesisDirectory = "/00000";
				boost::filesystem::create_directories(destination + nemesisDirectory);

				FakeFinalizationPoint(destination, 2);
				SetIndexFinalizationPoint(destination, 1);

				return std::make_unique<FileProofStorage>(destination);
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
			std::unique_ptr<test::TempDirectoryGuard> m_pTempDirectoryGuard;
			std::unique_ptr<FileProofStorage> m_pStorage;
		};

		// endregion

		// region test utils

		auto GenerateProof(size_t numVotes, FinalizationPoint finalizationPoint) {
			FinalizationProof proof;

			auto hash = test::GenerateRandomByteArray<Hash256>();
			crypto::StepIdentifier stepIdentifier = { finalizationPoint.unwrap(), 123, 456 };
			for (auto i = 0u; i < numVotes; ++i)
				proof.push_back(test::CreateMessage(stepIdentifier, hash));

			return proof;
		}

		auto PrepareStorageWithProofs(size_t numProofs) {
			StorageContext context;
			for (auto i = 2u; i <= numProofs; ++i) {
				auto newProof = GenerateProof(5, FinalizationPoint(i));
				context->saveProof(Height(100 + 2 * i), newProof);
			}

			return context;
		}

		// endregion

		// region assert helpers

		void AssertStorageIndexes(const FileProofStorage& storage, FinalizationPoint finalizationPoint, Height height) {
			EXPECT_EQ(finalizationPoint, storage.finalizationPoint());
			EXPECT_EQ(height, storage.finalizedHeight());
		}

		void AssertVoteProof(const model::FinalizationMessage& expectedMessage, const model::VoteProof& voteProof) {
			EXPECT_EQ(expectedMessage.Signature, voteProof.Signature);
			EXPECT_EQ(expectedMessage.SortitionHashProof.Gamma, voteProof.SortitionHashProof.Gamma);
			EXPECT_EQ(expectedMessage.SortitionHashProof.VerificationHash, voteProof.SortitionHashProof.VerificationHash);
			EXPECT_EQ(expectedMessage.SortitionHashProof.Scalar, voteProof.SortitionHashProof.Scalar);
		}

		void AssertSerializedProof(
				const FinalizationProof& expectedProof,
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

		void AssertHashes(
				const model::HeightHashPairRange& hashHeightPairRange,
				std::initializer_list<model::HeightHashPair> expectedHashHeightPairs) {
			ASSERT_EQ(expectedHashHeightPairs.size(), hashHeightPairRange.size());

			auto iter = hashHeightPairRange.cbegin();
			for (const auto& expectedPair : expectedHashHeightPairs) {
				EXPECT_EQ(expectedPair.Height, iter->Height);
				EXPECT_EQ(expectedPair.Hash, iter->Hash);
				++iter;
			}
		}

		// endregion
	}

	// region finalizationPoint + finalizedHeight

	TEST(TEST_CLASS, FinalizatationPointReturnsZeroWhenIndexDoesNotExist) {
		// Arrange:
		test::TempDirectoryGuard tempDirectoryGuard;
		FileProofStorage storage(tempDirectoryGuard.name());

		// Act:
		auto finalizationPoint = storage.finalizationPoint();

		// Assert:
		EXPECT_EQ(FinalizationPoint(), finalizationPoint);
	}

	TEST(TEST_CLASS, FinalizedHeightReturnsZeroWhenIndexDoesNotExist) {
		// Arrange:
		test::TempDirectoryGuard tempDirectoryGuard;
		FileProofStorage storage(tempDirectoryGuard.name());

		// Act:
		auto height = storage.finalizedHeight();

		// Assert:
		EXPECT_EQ(Height(), height);
	}

	// endregion

	// region saveProof - success

	TEST(TEST_CLASS, SavingProofWithFinalizationPointHigherThanCurrentFinalizationPointAltersFinalizationIndexes) {
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

	TEST(TEST_CLASS, CanLoadNewlySavedProof) {
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

	TEST(TEST_CLASS, SaveProofUsesDataFromFirstMessage) {
		// Arrange:
		auto pStorage = PrepareStorageWithProofs(10);
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();

		FinalizationProof proof;
		proof.push_back(test::CreateMessage({ 11, 78, 654 }, hash1));
		proof.push_back(test::CreateMessage({ 42, 90, 321 }, hash2));

		// Act:
		pStorage->saveProof(Height(123), proof);

		auto pProof = pStorage->loadProof(FinalizationPoint(11));

		// Assert:
		AssertStorageIndexes(*pStorage, FinalizationPoint(11), Height(123));
		EXPECT_EQ(hash1, pProof->FinalizedHash);
		EXPECT_EQ(Height(123), pProof->FinalizedHeight);
		EXPECT_EQ(crypto::StepIdentifier({ 11, 78, 654 }), pProof->StepIdentifier);

		const auto* pVoteProof = pProof->VoteProofsPtr();
		for (const auto& pMessage : proof)
			AssertVoteProof(*pMessage, *pVoteProof++);

		// - finalization point from second message is not accessible
		EXPECT_THROW(pStorage->loadProof(FinalizationPoint(42)), catapult_invalid_argument);
	}

	// endregion

	// region saveProof - failure

	namespace {
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
	}

	TEST(TEST_CLASS, CannotSaveEmptyProof) {
		// Arrange:
		auto pStorage = PrepareStorageWithProofs(10);
		FinalizationProof proof;

		// Act + Assert:
		EXPECT_THROW(pStorage->saveProof(Height(123), proof), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotSaveProofWithFinalizationPointLessThanCurrentFinalizationPoint) {
		AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(1));
		AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(9));
	}

	TEST(TEST_CLASS, CannotSaveProofMoreThanOneFinalizationPointBeyondCurrentFinalizationPoint) {
		AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(12));
		AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(110));
	}

	TEST(TEST_CLASS, CannotSaveProofWithHeightLessOrEqualToCurrentHeight) {
		AssertCannotSaveProofAtHeight(Height(109));
		AssertCannotSaveProofAtHeight(Height(120));
	}

	TEST(TEST_CLASS, CanSaveProofWithHeightGreaterThanCurrentHeight) {
		AssertCanSaveProofAtHeight(Height(121));
		AssertCanSaveProofAtHeight(Height(125));
		AssertCanSaveProofAtHeight(Height(150));
	}

	// endregion

	// region loadFinalizedHashesFrom

	TEST(TEST_CLASS, CanLoadHashesAtFinalizationPointGreaterThanCurrentFinalizationPoint) {
		// Arrange:
		auto pStorage = PrepareStorageWithProofs(10);

		// Act:
		auto hashes = pStorage->loadFinalizedHashesFrom(FinalizationPoint(11), 100);

		// Assert:
		AssertHashes(hashes, {});
	}

	TEST(TEST_CLASS, LoadingHashesAtFinalizationPointZeroReturnsEmptyRange) {
		// Arrange:
		auto pStorage = PrepareStorageWithProofs(10);

		// Act:
		auto hashes = pStorage->loadFinalizedHashesFrom(FinalizationPoint(0), 100);

		// Assert:
		AssertHashes(hashes, {});
	}

	// endregion

	// region loadProof

	TEST(TEST_CLASS, CanLoadProofAtFinalizationPointLessThanCurrentFinalizationPoint) {
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

	TEST(TEST_CLASS, CannotLoadProofAtFinalizationPointGreaterThanCurrentFinalizationPoint) {
		// Arrange:
		auto pStorage = PrepareStorageWithProofs(10);

		// Act + Assert:
		EXPECT_THROW(pStorage->loadProof(FinalizationPoint(11)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanLoadMultipleSavedProofs) {
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
}}

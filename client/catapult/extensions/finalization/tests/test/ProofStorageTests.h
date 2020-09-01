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
#include "finalization/src/model/FinalizationProofUtils.h"
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

					FakeFinalizationHeightMapping(destination, 2);
					SetIndexFinalizationPoint(destination, FinalizationPoint(1));
				}

				return TTraits::CreateStorage(destination);
			}

			static void SetIndexFinalizationPoint(const std::string& destination, FinalizationPoint finalizationPoint) {
				io::RawFile indexFile(destination + "/proof.index.dat", io::OpenMode::Read_Write);
				io::Write64(indexFile, finalizationPoint.unwrap());
				io::Write64(indexFile, 0);
				indexFile.write(Hash256());
			}

			static void FakeFinalizationHeightMapping(const std::string& destination, uint64_t numFinalizationPoints) {
				const std::string nemesisDirectory = "/00000";
				const std::string nemesisHashFilename = destination + nemesisDirectory + "/proof.heights.dat";

				std::vector<uint8_t> hashesBuffer(numFinalizationPoints * sizeof(Height));
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

		static auto GenerateProof(size_t numMessages, FinalizationPoint point, Height height) {
			std::vector<std::shared_ptr<const model::FinalizationMessage>> messages;
			auto hash = GenerateRandomByteArray<Hash256>();

			model::StepIdentifier stepIdentifier{ point, model::FinalizationStage::Precommit };
			for (auto i = 0u; i < numMessages; ++i)
				messages.push_back(CreateMessage(stepIdentifier, hash));

			return model::CreateFinalizationProof({ point, height, hash }, messages);
		}

		static auto PrepareStorageWithProofs(size_t numProofs) {
			StorageContext context(PreparationMode::Default);
			for (auto i = 2u; i <= numProofs; ++i) {
				auto pProof = GenerateProof(5, FinalizationPoint(i), Height(100 + 2 * i));
				context->saveProof(*pProof);
			}

			return context;
		}

		// endregion

		// region assert helpers

		static void AssertStorageStatistics(const io::ProofStorage& storage, const model::FinalizationStatistics& statistics) {
			auto storageStatistics = storage.statistics();
			EXPECT_EQ(statistics.Point, storageStatistics.Point);
			EXPECT_EQ(statistics.Height, storageStatistics.Height);
			EXPECT_EQ(statistics.Hash, storageStatistics.Hash);
		}

		static void AssertSerializedProof(const model::FinalizationProof& expectedProof, const model::FinalizationProof& actualProof) {
			EXPECT_EQ(expectedProof.Point, actualProof.Point);
			EXPECT_EQ(expectedProof.Height, actualProof.Height);
			EXPECT_EQ(expectedProof.Hash, actualProof.Hash);

			EXPECT_EQ(expectedProof, actualProof);
		}

		// endregion

	public:
		// region statistics

		static void AssertStatisticsReturnsEmptyStatisticsWhenIndexDoesNotExist() {
			// Arrange:
			StorageContext pStorage(PreparationMode::None);

			// Act + Assert:
			AssertStorageStatistics(*pStorage, model::FinalizationStatistics());
		}

		// endregion

		// region saveProof - success

		static void AssertSavingProofWithFinalizationPointHigherThanCurrentFinalizationPointAltersFinalizationIndexes() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			auto pProof1 = GenerateProof(3, FinalizationPoint(11), Height(123));
			auto pProof2 = GenerateProof(3, FinalizationPoint(12), Height(125));
			pStorage->saveProof(*pProof1);

			// Sanity:
			AssertStorageStatistics(*pStorage, { FinalizationPoint(11), Height(123), pProof1->Hash });

			// Act:
			pStorage->saveProof(*pProof2);

			// Assert:
			AssertStorageStatistics(*pStorage, { FinalizationPoint(12), Height(125), pProof2->Hash });
		}

		static void AssertCanLoadNewlySavedProof() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			auto pProof = GenerateProof(3, FinalizationPoint(11), Height(123));

			// Act:
			pStorage->saveProof(*pProof);

			auto pLoadedProof = pStorage->loadProof(FinalizationPoint(11));

			// Assert:
			AssertStorageStatistics(*pStorage, { FinalizationPoint(11), Height(123), pProof->Hash });
			AssertSerializedProof(*pProof, *pLoadedProof);
		}

		// endregion

		// region saveProof - failure

	private:
		static void AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint newFinalizationPoint) {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			auto pProof = GenerateProof(3, newFinalizationPoint, Height(123));

			// Act + Assert:
			EXPECT_THROW(pStorage->saveProof(*pProof), catapult_invalid_argument);
		}

		static void AssertCanSaveProofAtFinalizationPoint(FinalizationPoint newFinalizationPoint) {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);
			auto pProof = GenerateProof(3, newFinalizationPoint, Height(123));

			// Act + Assert:
			EXPECT_NO_THROW(pStorage->saveProof(*pProof));
		}

		static void AssertCannotSaveProofAtHeight(Height newFinalizedHeight) {
			// Arrange: prepare storage with proofs for heights 104-120
			auto pStorage = PrepareStorageWithProofs(10);
			auto pProof = GenerateProof(3, FinalizationPoint(11), newFinalizedHeight);

			// Act + Assert:
			EXPECT_THROW(pStorage->saveProof(*pProof), catapult_invalid_argument);
		}

		static void AssertCanSaveProofAtHeight(Height newFinalizedHeight) {
			// Arrange: prepare storage with proofs for heights 104-120
			auto pStorage = PrepareStorageWithProofs(10);
			auto pProof = GenerateProof(3, FinalizationPoint(11), newFinalizedHeight);

			// Act + Assert:
			EXPECT_NO_THROW(pStorage->saveProof(*pProof));
		}

	public:
		static void AssertCannotSaveProofWithFinalizationPointLessThanCurrentFinalizationPoint() {
			AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(1));
			AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(8));
			AssertCannotSaveProofAtFinalizationPoint(FinalizationPoint(9));
		}

		static void AssertCanSaveProofWithFinalizationPointGreaterThanOrEqualToCurrentFinalizationPoint() {
			AssertCanSaveProofAtFinalizationPoint(FinalizationPoint(10));
			AssertCanSaveProofAtFinalizationPoint(FinalizationPoint(11));
			AssertCanSaveProofAtFinalizationPoint(FinalizationPoint(110));
			AssertCanSaveProofAtFinalizationPoint(FinalizationPoint(150));
		}

		static void AssertCannotSaveProofWithHeightLessThanCurrentHeight() {
			AssertCannotSaveProofAtHeight(Height(109));
			AssertCannotSaveProofAtHeight(Height(119));
		}

		static void AssertCanSaveProofWithHeightGreaterThanOrEqualToCurrentHeight() {
			AssertCanSaveProofAtHeight(Height(120));
			AssertCanSaveProofAtHeight(Height(125));
			AssertCanSaveProofAtHeight(Height(150));
		}

		// endregion

		// region loadProof(point)

		static void AssertCanLoadProofAtFinalizationPointLessThanCurrentFinalizationPoint() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto pProof1 = GenerateProof(3, FinalizationPoint(11), Height(123));
			pStorage->saveProof(*pProof1);

			auto pProof2 = GenerateProof(3, FinalizationPoint(12), Height(125));
			pStorage->saveProof(*pProof2);

			// Act:
			auto pLoadedProof = pStorage->loadProof(FinalizationPoint(11));

			// Assert:
			AssertStorageStatistics(*pStorage, { FinalizationPoint(12), Height(125), pProof2->Hash });
			AssertSerializedProof(*pProof1, *pLoadedProof);
		}

		static void AssertCannotLoadProofAtFinalizationPointZero() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			// Act + Assert:
			EXPECT_THROW(pStorage->loadProof(FinalizationPoint(0)), catapult_invalid_argument);
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

			auto pProof1 = GenerateProof(3, FinalizationPoint(11), Height(123));
			auto pProof2 = GenerateProof(3, FinalizationPoint(12), Height(125));
			pStorage->saveProof(*pProof1);
			pStorage->saveProof(*pProof2);

			// Act:
			auto pLoadedProof1 = pStorage->loadProof(FinalizationPoint(11));
			auto pLoadedProof2 = pStorage->loadProof(FinalizationPoint(12));

			// Assert:
			AssertStorageStatistics(*pStorage, { FinalizationPoint(12), Height(125), pProof2->Hash });
			AssertSerializedProof(*pProof1, *pLoadedProof1);
			AssertSerializedProof(*pProof2, *pLoadedProof2);
		}

		// endregion

		// region loadProof(height)

		static void AssertCanLoadProofAtFinalizedHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto pProof = GenerateProof(3, FinalizationPoint(11), Height(123));
			pStorage->saveProof(*pProof);

			// Act:
			auto pLoadedProof = pStorage->loadProof(Height(123));

			// Assert:
			AssertStorageStatistics(*pStorage, { FinalizationPoint(11), Height(123), pProof->Hash });
			AssertSerializedProof(*pProof, *pLoadedProof);
		}

		static void AssertCanLoadProofAtHeightLessThanCurrentFinalizedHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto pProof1 = GenerateProof(3, FinalizationPoint(11), Height(123));
			pStorage->saveProof(*pProof1);

			auto pProof2 = GenerateProof(3, FinalizationPoint(12), Height(125));
			pStorage->saveProof(*pProof2);

			// Act:
			auto pLoadedProof = pStorage->loadProof(Height(123));

			// Assert: finalized proof is proof2, but requested proof is correctly loaded
			AssertStorageStatistics(*pStorage, { FinalizationPoint(12), Height(125), pProof2->Hash });
			AssertSerializedProof(*pProof1, *pLoadedProof);
		}

		static void AssertLoadProofAtHeightLoadsMostRecentProof() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto pProof1 = GenerateProof(3, FinalizationPoint(11), Height(123));
			pStorage->saveProof(*pProof1);

			auto pProof2 = GenerateProof(3, FinalizationPoint(12), Height(123));
			pStorage->saveProof(*pProof2);

			auto pProof3 = GenerateProof(3, FinalizationPoint(13), Height(123));
			pStorage->saveProof(*pProof3);

			// Act:
			auto pLoadedProof = pStorage->loadProof(Height(123));

			// Assert:
			AssertStorageStatistics(*pStorage, { FinalizationPoint(13), Height(123), pProof3->Hash });
			AssertSerializedProof(*pProof3, *pLoadedProof);
		}

		static void AssertCannotLoadProofAtHeightZero() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(3);

			// Act + Assert:
			EXPECT_THROW(pStorage->loadProof(Height(0)), catapult_invalid_argument);
		}

		static void AssertCannotLoadProofAtHeightGreaterThanCurrentFinalizedHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto pProof = GenerateProof(3, FinalizationPoint(11), Height(123));
			pStorage->saveProof(*pProof);

			// Act + Assert:
			EXPECT_THROW(pStorage->loadProof(Height(124)), catapult_invalid_argument);
		}

		static void AssertCannotLoadProofAtHeightWithoutProof() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto pProof1 = GenerateProof(3, FinalizationPoint(11), Height(123));
			pStorage->saveProof(*pProof1);

			auto pProof2 = GenerateProof(3, FinalizationPoint(12), Height(125));
			pStorage->saveProof(*pProof2);

			// Act:
			auto pLoadedProof = pStorage->loadProof(Height(124));

			// Assert:
			EXPECT_FALSE(!!pLoadedProof);
		}

		static void AssertCanLoadProofAtHeightOutsideSingleBatch() {
			// Arrange:
			auto pStorage = PrepareStorageWithProofs(10);

			auto pProof1 = GenerateProof(3, FinalizationPoint(11), Height(123));
			pStorage->saveProof(*pProof1);

			Hash256 lastHash;
			for (auto i = 0u; i < 200; ++i) {
				auto pProof2 = GenerateProof(1, FinalizationPoint(12 + i), Height(130 + 2 * i));
				pStorage->saveProof(*pProof2);

				if (199 == i)
					lastHash = pProof2->Hash;
			}

			// Act:
			auto pLoadedProof = pStorage->loadProof(Height(123));

			// Assert: finalized proof is proof2, but requested proof is correctly loaded
			AssertStorageStatistics(*pStorage, { FinalizationPoint(211), Height(528), lastHash });
			AssertSerializedProof(*pProof1, *pLoadedProof);
		}

		// endregion
	};
}}

// region MAKE/DEFINE TESTs

#define MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::ProofStorageTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_PROOF_STORAGE_TESTS(TRAITS_NAME) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, StatisticsReturnsEmptyStatisticsWhenIndexDoesNotExist) \
	\
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, SavingProofWithFinalizationPointHigherThanCurrentFinalizationPointAltersFinalizationIndexes) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadNewlySavedProof) \
	\
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotSaveProofWithFinalizationPointLessThanCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanSaveProofWithFinalizationPointGreaterThanOrEqualToCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotSaveProofWithHeightLessThanCurrentHeight) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanSaveProofWithHeightGreaterThanOrEqualToCurrentHeight) \
	\
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadProofAtFinalizationPointLessThanCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotLoadProofAtFinalizationPointZero) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotLoadProofAtFinalizationPointGreaterThanCurrentFinalizationPoint) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadMultipleSavedProofs) \
	\
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadProofAtFinalizedHeight) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadProofAtHeightLessThanCurrentFinalizedHeight) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, LoadProofAtHeightLoadsMostRecentProof) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotLoadProofAtHeightZero) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotLoadProofAtHeightGreaterThanCurrentFinalizedHeight) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CannotLoadProofAtHeightWithoutProof) \
	MAKE_PROOF_STORAGE_TEST(TRAITS_NAME, CanLoadProofAtHeightOutsideSingleBatch)

// endregion

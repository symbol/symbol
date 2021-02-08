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

#include "catapult/local/importer/ChainImporter.h"
#include "catapult/cache/SupplementalDataStorage.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/io/FileStream.h"
#include "catapult/subscribers/StateChangeInfo.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/local/BlockStateHash.h"
#include "tests/test/local/FilechainTestUtils.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS ChainImporterTests

	namespace {
		enum class SubscriberMode { None, Block_Change, State_Change, Finalization };
		enum class CacheDatabaseMode { Enabled, Disabled };

		// region subscriber mocks

		class MockBlockChangeSubscriber : public io::BlockChangeSubscriber {
		public:
			explicit MockBlockChangeSubscriber(std::vector<Height>& heights) : m_heights(heights)
			{}

		public:
			void notifyBlock(const model::BlockElement& blockElement) override {
				m_heights.push_back(blockElement.Block.Height);
			}

			void notifyDropBlocksAfter(Height) override {
				CATAPULT_THROW_RUNTIME_ERROR("notifyDropBlocksAfter - not supported in mock");
			}

		private:
			std::vector<Height>& m_heights;
		};

		class MockStateChangeSubscriber : public subscribers::StateChangeSubscriber {
		public:
			explicit MockStateChangeSubscriber(std::vector<Height>& heights) : m_heights(heights)
			{}

		public:
			void notifyScoreChange(const model::ChainScore&) override
			{}

			void notifyStateChange(const subscribers::StateChangeInfo& stateChangeInfo) override {
				m_heights.push_back(stateChangeInfo.Height);
			}

		private:
			std::vector<Height>& m_heights;
		};

		class MockFinalizationSubscriber : public subscribers::FinalizationSubscriber {
		public:
			explicit MockFinalizationSubscriber(std::vector<Height>& heights) : m_heights(heights)
			{}

		public:
			void notifyFinalizedBlock(const model::FinalizationRound&, Height height, const Hash256&) override {
				m_heights.push_back(height);
			}

		private:
			std::vector<Height>& m_heights;
		};

		// endregion

		// region FinalizationProofHeader

#pragma pack(push, 1)

		struct FinalizationProofHeader : public model::SizePrefixedEntity {
			uint32_t Version;
			model::FinalizationRound Round;
			catapult::Height Height;
			Hash256 Hash;
		};

#pragma pack(pop)

		// endregion

		// region test context

		class TestContext {
		public:
			TestContext() : m_dataDirectory(m_tempDir.name()) {
				test::PrepareStorage(m_dataDirectory.rootDir().str());
			}

		public:
			const auto& blockChangeHeights() const {
				return m_blockChangeHeights;
			}

			const auto& stateChangeHeights() const {
				return m_stateChangeHeights;
			}

			const auto& finalizationHeights() const {
				return m_finalizationHeights;
			}

		public:
			Height readSupplementalHeight() const {
				auto supplementalFileStream = io::FileStream(io::RawFile(
						m_dataDirectory.dir("state").file("supplemental.dat"),
						io::OpenMode::Read_Only));

				cache::SupplementalData supplementalData;
				Height chainHeight;
				cache::LoadSupplementalData(supplementalFileStream, supplementalData, chainHeight);
				return chainHeight;
			}

		public:
			void createSerializedState() {
				auto stateDir = m_dataDirectory.dir("state");
				stateDir.create();
				io::RawFile(stateDir.file("supplemental.dat"), io::OpenMode::Read_Write);
			}

			void seedBlocks(uint64_t numBlocks) {
				auto recipients = test::GenerateRandomAddresses(numBlocks);

				// generate block per every recipient, each with random number of transactions
				auto height = 2u;
				std::mt19937_64 rnd;
				auto nemesisKeyPairs = test::GetNemesisKeyPairs();

				io::FileBlockStorage storage(m_dataDirectory.rootDir().str(), test::File_Database_Batch_Size);
				for (auto i = 0u; i < numBlocks; ++i) {
					auto timeSpacing = utils::TimeSpan::FromMinutes(1);
					auto blockWithAttributes = test::CreateBlock(nemesisKeyPairs, recipients[i], rnd, height, timeSpacing);
					storage.saveBlock(test::BlockToBlockElement(*blockWithAttributes.pBlock));
					++height;
				}
			}

			void seedProofs(uint64_t numProofs) {
				io::FileDatabase proofFileDatabase(m_dataDirectory.rootDir(), { test::File_Database_Batch_Size, ".proof" });
				for (auto i = 0u; i < numProofs; ++i) {
					auto pProofStream = proofFileDatabase.outputStream(2 + i);

					FinalizationProofHeader proofHeader;
					test::FillWithRandomData(proofHeader);
					proofHeader.Height = Height(3 + 2 * i);

					pProofStream->write({ reinterpret_cast<const uint8_t*>(&proofHeader), sizeof(FinalizationProofHeader) });
				}
			}

		public:
			void boot(
					SubscriberMode subscriberMode = SubscriberMode::None,
					CacheDatabaseMode cacheDatabaseMode = CacheDatabaseMode::Disabled) {
				auto config = test::CreateCatapultConfigurationWithNemesisPluginExtensions(m_dataDirectory.rootDir().str());
				test::AddRecoveryPluginExtensions(const_cast<config::ExtensionsConfiguration&>(config.Extensions));

				if (CacheDatabaseMode::Enabled == cacheDatabaseMode) {
					const_cast<model::BlockChainConfiguration&>(config.BlockChain).EnableVerifiableState = true;
					const_cast<config::NodeConfiguration&>(config.Node).EnableCacheDatabaseStorage = true;

					test::ModifyNemesis(m_dataDirectory.rootDir().str(), [&config](auto& nemesisBlock, const auto& nemesisBlockElement) {
						nemesisBlock.StateHash = test::CalculateNemesisStateHash(nemesisBlockElement, config);
					});
				}

				auto pBootstrapper = std::make_unique<extensions::ProcessBootstrapper>(
						std::move(config),
						m_dataDirectory.dir("resources").str(),
						extensions::ProcessDisposition::Production,
						"ChainImporterTests");
				pBootstrapper->loadExtensions();

				auto& subscriptionManager = pBootstrapper->subscriptionManager();
				if (SubscriberMode::Block_Change == subscriberMode)
					subscriptionManager.addBlockChangeSubscriber(std::make_unique<MockBlockChangeSubscriber>(m_blockChangeHeights));
				else if (SubscriberMode::State_Change == subscriberMode)
					subscriptionManager.addStateChangeSubscriber(std::make_unique<MockStateChangeSubscriber>(m_stateChangeHeights));
				else if (SubscriberMode::Finalization == subscriberMode)
					subscriptionManager.addFinalizationSubscriber(std::make_unique<MockFinalizationSubscriber>(m_finalizationHeights));

				CreateChainImporter(std::move(pBootstrapper));
			}

		public:
			void assertStateChangeIndex(uint64_t expected) {
				auto stateChangeDir = m_dataDirectory.spoolDir("state_change");
				EXPECT_EQ(expected, io::IndexFile(stateChangeDir.file("index_server.dat")).get());
				EXPECT_EQ(expected, io::IndexFile(stateChangeDir.file("index.dat")).get());
			}

		private:
			test::TempDirectoryGuard m_tempDir;
			config::CatapultDataDirectory m_dataDirectory;

			std::vector<Height> m_blockChangeHeights;
			std::vector<Height> m_stateChangeHeights;
			std::vector<Height> m_finalizationHeights;
		};

		// endregion
	}

	// region nemesis

	TEST(TEST_CLASS, CannotImportNemesisWhenSerializedStateIsPresent) {
		// Arrange:
		TestContext context;
		context.createSerializedState();

		// Act + Assert: exception is always rethrown by CreateAndBootHost as catapult_runtime_error
		EXPECT_THROW(context.boot(), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanImportNemesisWithoutSubscribers) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		context.assertStateChangeIndex(2);

		EXPECT_TRUE(context.blockChangeHeights().empty());
		EXPECT_TRUE(context.stateChangeHeights().empty());
		EXPECT_TRUE(context.finalizationHeights().empty());
	}

	TEST(TEST_CLASS, CanImportNemesisWithBlockChangeSubscriber) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot(SubscriberMode::Block_Change);

		// Assert:
		context.assertStateChangeIndex(2);

		EXPECT_EQ(std::vector<Height>({ Height(1) }), context.blockChangeHeights());
		EXPECT_TRUE(context.stateChangeHeights().empty());
		EXPECT_TRUE(context.finalizationHeights().empty());
	}

	TEST(TEST_CLASS, CanImportNemesisWithStateChangeSubscriber) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot(SubscriberMode::State_Change);

		// Assert:
		context.assertStateChangeIndex(2);

		EXPECT_TRUE(context.blockChangeHeights().empty());
		EXPECT_EQ(std::vector<Height>({ Height(1) }), context.stateChangeHeights());
		EXPECT_TRUE(context.finalizationHeights().empty());
	}

	TEST(TEST_CLASS, CanImportNemesisWithFinalizationSubscriber) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot(SubscriberMode::Finalization);

		// Assert:
		context.assertStateChangeIndex(2);

		EXPECT_TRUE(context.blockChangeHeights().empty());
		EXPECT_TRUE(context.stateChangeHeights().empty());
		EXPECT_EQ(std::vector<Height>({ Height(1) }), context.finalizationHeights());
	}

	// endregion

	// region chain

	namespace {
		std::vector<Height> GetHeightRange(Height startHeight, Height endHeight) {
			std::vector<Height> heights;
			for (auto height = startHeight; height <= endHeight; height = height + Height(1))
				heights.push_back(height);

			return heights;
		}
	}

	TEST(TEST_CLASS, CanImportChainWithoutSubscribers) {
		// Arrange:
		TestContext context;
		context.seedBlocks(5);
		context.seedProofs(2);

		// Act:
		context.boot();

		// Assert:
		context.assertStateChangeIndex(2 * 6);

		EXPECT_TRUE(context.blockChangeHeights().empty());
		EXPECT_TRUE(context.stateChangeHeights().empty());
		EXPECT_TRUE(context.finalizationHeights().empty());
	}

	TEST(TEST_CLASS, CanImportChainWithBlockChangeSubscriber) {
		// Arrange:
		TestContext context;
		context.seedBlocks(5);
		context.seedProofs(2);

		// Act:
		context.boot(SubscriberMode::Block_Change);

		// Assert:
		context.assertStateChangeIndex(2 * 6);

		EXPECT_EQ(GetHeightRange(Height(1), Height(6)), context.blockChangeHeights());
		EXPECT_TRUE(context.stateChangeHeights().empty());
		EXPECT_TRUE(context.finalizationHeights().empty());
	}

	TEST(TEST_CLASS, CanImportChainWithStateChangeSubscriber) {
		// Arrange:
		TestContext context;
		context.seedBlocks(5);
		context.seedProofs(2);

		// Act:
		context.boot(SubscriberMode::State_Change);

		// Assert:
		context.assertStateChangeIndex(2 * 6);

		EXPECT_TRUE(context.blockChangeHeights().empty());
		EXPECT_EQ(GetHeightRange(Height(1), Height(6)), context.stateChangeHeights());
		EXPECT_TRUE(context.finalizationHeights().empty());
	}

	TEST(TEST_CLASS, CanImportChainWithFinalizationSubscriber) {
		// Arrange:
		TestContext context;
		context.seedBlocks(5);
		context.seedProofs(2);

		// Act:
		context.boot(SubscriberMode::Finalization);

		// Assert:
		context.assertStateChangeIndex(2 * 6);

		EXPECT_TRUE(context.blockChangeHeights().empty());
		EXPECT_TRUE(context.stateChangeHeights().empty());
		EXPECT_EQ(std::vector<Height>({ Height(1), Height(3), Height(5) }), context.finalizationHeights());
	}

	// endregion

	// region disk persistence

	namespace {
		void AssertSupplementalDataIsWrittenToDisk(CacheDatabaseMode cacheDatabaseMode) {
			// Arrange:
			TestContext context;
			context.seedBlocks(5);
			context.seedProofs(2);

			// Act:
			context.boot(SubscriberMode::None, cacheDatabaseMode);

			// Assert:
			EXPECT_EQ(Height(6), context.readSupplementalHeight());
		}
	}

	TEST(TEST_CLASS, SupplementalDataIsWrittenToDisk_CacheDatabaseDisabled) {
		AssertSupplementalDataIsWrittenToDisk(CacheDatabaseMode::Disabled);
	}

	TEST(TEST_CLASS, SupplementalDataIsWrittenToDisk_CacheDatabaseEnabled) {
		AssertSupplementalDataIsWrittenToDisk(CacheDatabaseMode::Enabled);
	}

	// endregion
}}

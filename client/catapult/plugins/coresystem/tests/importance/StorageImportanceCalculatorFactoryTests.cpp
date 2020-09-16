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

#include "src/importance/StorageImportanceCalculatorFactory.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/io/IndexFile.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace importance {

#define TEST_CLASS StorageImportanceCalculatorFactoryTests

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(2222);

		enum class SeedStrategy { Zero, Default };

		// region MockImportanceCalculator

		class MockImportanceCalculator final : public ImportanceCalculator {
		public:
			explicit MockImportanceCalculator(SeedStrategy seedStrategy) : m_seedStrategy(seedStrategy)
			{}

		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				const auto& highValueAddresses = cache.highValueAccounts().addresses();
				for (const auto& address : highValueAddresses) {
					auto seed = SeedStrategy::Zero == m_seedStrategy ? 0 : address[0] + importanceHeight.unwrap();
					auto accountStateIter = cache.find(address);

					accountStateIter.get().ImportanceSnapshots.set(Importance(seed * seed), importanceHeight);
					accountStateIter.get().ActivityBuckets.update(importanceHeight, [seed](auto& bucket) {
						bucket.RawScore = seed * seed * seed;
					});
				}
			}

		private:
			SeedStrategy m_seedStrategy;
		};

		// endregion

		// region CacheHolder

		class CacheHolder {
		public:
			CacheHolder()
					: m_cache(
							cache::CacheConfiguration(),
							test::CreateDefaultAccountStateCacheOptions(MosaicId(1111), Harvesting_Mosaic_Id))
					, m_delta(m_cache.createDelta())
			{}

		public:
			auto& get() {
				return *m_delta;
			}

			const auto& get(const Address& address) {
				return get().find(address).get();
			}

		public:
			void addAccounts(const std::vector<Address>& addresses, Amount balance) {
				for (const auto& address : addresses) {
					m_delta->addAccount(address, Height(1));
					m_delta->find(address).get().Balances.credit(Harvesting_Mosaic_Id, balance);
				}

				m_delta->updateHighValueAccounts(Height(1));
			}

			std::vector<Address> addAccounts(size_t count, Amount balance) {
				auto addresses = test::GenerateRandomDataVector<Address>(count);
				for (auto i = 0u; i < count; ++i)
					addresses[i][0] = static_cast<uint8_t>(i + 1);

				addAccounts(addresses, balance);
				return addresses;
			}

		public:
			void assertImportanceInputs(
					const std::vector<Address>& addresses,
					model::ImportanceHeight importanceHeight,
					SeedStrategy strategy) {
				auto i = 0u;
				for (const auto& address : addresses) {
					const auto& accountState = get(address);
					auto seed = i + 1 + importanceHeight.unwrap();

					std::ostringstream out;
					out << "address at " << i << " (seed = " << seed << ")";
					auto message = out.str();

					// Assert:
					EXPECT_EQ(importanceHeight, accountState.ImportanceSnapshots.height()) << message;
					EXPECT_EQ(GetExpectedImportance(seed, strategy), accountState.ImportanceSnapshots.current()) << message;

					EXPECT_EQ(importanceHeight, accountState.ActivityBuckets.begin()->StartHeight) << message;
					EXPECT_EQ(GetExpectedRawScore(seed, strategy), accountState.ActivityBuckets.begin()->RawScore) << message;

					++i;
				}
			}

			void assertBalances(const std::vector<Address>& addresses, Amount expectedBalance) {
				auto i = 0u;
				for (const auto& address : addresses) {
					const auto& accountState = get(address);

					// Assert:
					EXPECT_EQ(expectedBalance, accountState.Balances.get(Harvesting_Mosaic_Id)) << "address at " << i;

					++i;
				}
			}

		private:
			static Importance GetExpectedImportance(size_t seed, SeedStrategy strategy) {
				return Importance(SeedStrategy::Zero == strategy ? 0 : seed * seed);
			}

			static uint64_t GetExpectedRawScore(size_t seed, SeedStrategy strategy) {
				return SeedStrategy::Zero == strategy ? 0 : seed * seed * seed;
			}

		private:
			cache::AccountStateCache m_cache;
			cache::LockedCacheDelta<cache::AccountStateCacheDelta> m_delta;
		};

		// endregion

		// region TestContext

		class TestContext {
		public:
			auto createWriteCalculator() {
				return factory().createWriteCalculator(std::make_unique<MockImportanceCalculator>(SeedStrategy::Default));
			}

			auto createReadCalculator(SeedStrategy seedStrategy = SeedStrategy::Zero) {
				return factory().createReadCalculator(std::make_unique<MockImportanceCalculator>(seedStrategy));
			}

		public:
			size_t fileSize(const std::string& filename) {
				return boost::filesystem::file_size(boost::filesystem::path(m_tempDir.name()) / filename);
			}

			uint64_t loadIndexValue() {
				return indexFile().get();
			}

			void setIndexValue(uint64_t value) {
				indexFile().set(value);
			}

		public:
			void assertFiles(const std::unordered_set<std::string>& expectedFilenames) {
				EXPECT_EQ(1 + expectedFilenames.size(), test::CountFilesAndDirectories(m_tempDir.name()));

				for (const auto& filename : expectedFilenames)
					EXPECT_TRUE(boost::filesystem::exists(boost::filesystem::path(m_tempDir.name()) / filename)) << filename;
			}

		private:
			io::IndexFile indexFile() {
				return io::IndexFile((boost::filesystem::path(m_tempDir.name()) / "index.dat").generic_string());
			}

			StorageImportanceCalculatorFactory factory() {
				return StorageImportanceCalculatorFactory(config::CatapultDirectory(m_tempDir.name()));
			}

		private:
			test::TempDirectoryGuard m_tempDir;
		};

		// endregion
	}

	// region createWriteCalculator

	TEST(TEST_CLASS, WriteCalculatorDelegatesToDecoratedCalculator) {
		// Arrange:
		TestContext context;
		CacheHolder holder;
		auto addresses = holder.addAccounts(4, Amount(1000));

		// Sanity:
		holder.assertImportanceInputs(addresses, model::ImportanceHeight(), SeedStrategy::Zero);

		// Act:
		auto pCalculator = context.createWriteCalculator();
		pCalculator->recalculate(model::ImportanceHeight(7), holder.get());

		// Assert:
		holder.assertImportanceInputs(addresses, model::ImportanceHeight(7), SeedStrategy::Default);
	}

	TEST(TEST_CLASS, WriteCalculatorCanWriteSingleImportanceFile) {
		// Arrange:
		TestContext context;
		CacheHolder holder;
		holder.addAccounts(4, Amount(1000));

		// Act:
		auto pCalculator = context.createWriteCalculator();
		pCalculator->recalculate(model::ImportanceHeight(7), holder.get());

		// Assert:
		EXPECT_EQ(7u, context.loadIndexValue());
		context.assertFiles({ "0000000000000007.dat" });
	}

	TEST(TEST_CLASS, WriteCalculatorCanOverwriteFile) {
		// Arrange:
		TestContext context;
		auto pCalculator = context.createWriteCalculator();

		{
			CacheHolder holder;
			holder.addAccounts(4, Amount(1000));
			pCalculator->recalculate(model::ImportanceHeight(7), holder.get());
		}

		auto fileSize1 = context.fileSize("0000000000000007.dat");

		{
			// - need to use a separate cache to avoid setting multiple importances at same height
			CacheHolder holder;
			holder.addAccounts(5, Amount(500));

			// Act:
			pCalculator->recalculate(model::ImportanceHeight(7), holder.get());
		}

		auto fileSize2 = context.fileSize("0000000000000007.dat");

		// Assert:
		EXPECT_LT(fileSize1, fileSize2);

		EXPECT_EQ(7u, context.loadIndexValue());
		context.assertFiles({ "0000000000000007.dat" });
	}

	TEST(TEST_CLASS, WriteCalculatorCanWriteMultipleImportanceFiles) {
		// Arrange:
		TestContext context;
		CacheHolder holder;
		holder.addAccounts(4, Amount(1000));

		// Act:
		auto pCalculator = context.createWriteCalculator();
		pCalculator->recalculate(model::ImportanceHeight(5), holder.get());
		pCalculator->recalculate(model::ImportanceHeight(7), holder.get());
		pCalculator->recalculate(model::ImportanceHeight(9), holder.get());

		// Assert:
		EXPECT_EQ(9u, context.loadIndexValue());
		context.assertFiles({ "0000000000000005.dat", "0000000000000007.dat", "0000000000000009.dat" });
	}

	// endregion

	// region createReadCalculator

	TEST(TEST_CLASS, ReadCalculatorFailsWhenIndexFileIsNotPresent) {
		// Arrange:
		TestContext context;
		CacheHolder holder;
		holder.addAccounts(4, Amount(1000));

		// Act + Assert:
		auto pCalculator = context.createReadCalculator();
		EXPECT_THROW(pCalculator->recalculate(model::ImportanceHeight(7), holder.get()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, ReadCalculatorBypassesFileLoadingWhenIndexValueMatchesImportanceHeight) {
		// Arrange:
		TestContext context;
		context.setIndexValue(7);

		CacheHolder holder;
		auto addresses = holder.addAccounts(4, Amount(1000));

		// Act:
		auto pCalculator = context.createReadCalculator(SeedStrategy::Default);
		pCalculator->recalculate(model::ImportanceHeight(7), holder.get());

		// Assert: loading was skipped and importances are set by decorated calculator
		holder.assertImportanceInputs(addresses, model::ImportanceHeight(7), SeedStrategy::Default);
	}

	TEST(TEST_CLASS, ReadCalculatorFailsWhenMatchingImportanceFileIsNotPresent) {
		// Arrange:
		TestContext context;
		context.setIndexValue(8);

		CacheHolder holder;
		holder.addAccounts(4, Amount(1000));

		// Act + Assert: there is no importance file at height 7
		auto pCalculator = context.createReadCalculator();
		EXPECT_THROW(pCalculator->recalculate(model::ImportanceHeight(7), holder.get()), catapult_runtime_error);
	}

	// endregion

	// region roundtrip

	namespace {
		template<typename TSetup>
		void RunRoundtripTest(TSetup setup) {
			// Arrange:
			TestContext context;
			setup(context);

			std::vector<Address> addresses;
			{
				CacheHolder holder;
				addresses = holder.addAccounts(4, Amount(1000));

				auto pCalculator = context.createWriteCalculator();
				pCalculator->recalculate(model::ImportanceHeight(5), holder.get());
				pCalculator->recalculate(model::ImportanceHeight(7), holder.get());
				pCalculator->recalculate(model::ImportanceHeight(9), holder.get());
			}

			// Sanity:
			EXPECT_EQ(9u, context.loadIndexValue());
			context.assertFiles({ "0000000000000005.dat", "0000000000000007.dat", "0000000000000009.dat" });

			CacheHolder holder;
			holder.addAccounts(addresses, Amount(500));

			// Act:
			auto pCalculator = context.createReadCalculator();
			pCalculator->recalculate(model::ImportanceHeight(7), holder.get());

			// Assert: importance inputs were updated but other fields (e.g. balance) were not
			holder.assertImportanceInputs(addresses, model::ImportanceHeight(7), SeedStrategy::Default);
			holder.assertBalances(addresses, Amount(500));
		}
	}

	TEST(TEST_CLASS, CanRoundtripImportanceFromFileNotMatchingIndex) {
		RunRoundtripTest([](const auto&) {});
	}

	TEST(TEST_CLASS, CanRoundtripImportanceFromOverwrittenFileNotMatchingIndex) {
		RunRoundtripTest([](auto& context) {
			// Arrange: write file for height 7 that should be overwritten
			CacheHolder holder;
			holder.addAccounts(3, Amount(100));

			auto pCalculator = context.createWriteCalculator();
			pCalculator->recalculate(model::ImportanceHeight(7), holder.get());
		});
	}

	// endregion

}}

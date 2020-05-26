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

#include "plugins/services/hashcache/src/cache/HashCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/utils/SpinLock.h"
#include "tests/int/stress/test/StressThreadLogger.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/thread.hpp>
#include <random>

namespace catapult { namespace cache {

#define TEST_CLASS CacheIntegrityTests

	namespace {
		constexpr auto Transferable_Mosaic_Id = MosaicId(1234);
		constexpr auto Default_Cache_Options = test::CreateDefaultAccountStateCacheOptions();

		size_t GetNumIterations() {
			return test::GetStressIterationCount() ? 20'000 : 1'000;
		}

		size_t GetNumStressAccounts() {
			return test::GetStressIterationCount() ? 20'000 : 1'000;
		}

		Key GetKeyFromId(size_t id) {
			Key key{};
			for (auto i = 0u; i < Key::Size / 2; ++i) {
				key[2 * i] = static_cast<uint8_t>(id / 256);
				key[2 * i + 1] = static_cast<uint8_t>(id % 256);
			}

			return key;
		}

		void RunMultithreadedReadWriteTest(size_t numReaders) {
			// Arrange:
			// - note that there can only ever be a single writer at a time since only one copy can be outstanding at once
			AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
			std::vector<Amount> sums(numReaders);

			// Act: set up reader thread(s) that sum up all account balances
			boost::thread_group threads;
			for (auto r = 0u; r < numReaders; ++r) {
				threads.create_thread([&, r] {
					test::StressThreadLogger logger("reader thread " + std::to_string(r));

					for (auto i = 0u; i < GetNumIterations(); ++i) {
						logger.notifyIteration(i, GetNumIterations());

						while (true) {
							auto key = GetKeyFromId(i);
							auto view = cache.createView();
							auto accountStateIter = view->find(key);
							if (!accountStateIter.tryGet())
								continue;

							sums[r] = sums[r] + accountStateIter.get().Balances.get(Transferable_Mosaic_Id);
							break;
						}
					}
				});
			}

			// - set up a writer thread that adds accounts to the cache
			threads.create_thread([&] {
				test::StressThreadLogger logger("writer thread");

				auto delta = cache.createDelta();
				for (auto i = 0u; i < GetNumIterations(); ++i) {
					logger.notifyIteration(i, GetNumIterations());

					auto key = GetKeyFromId(i);
					delta->addAccount(key, Height(456));
					auto accountStateIter = delta->find(key);
					auto& accountState = accountStateIter.get();
					accountState.Balances.credit(Transferable_Mosaic_Id, Amount(i * 100'000));
					cache.commit();
				}
			});

			// - wait for all threads
			threads.join_all();

			// Assert: all accounts were added to the cache and the reader(s) calculated the correct sum
			EXPECT_EQ(GetNumIterations(), cache.createView()->size());

			auto expectedSum = Amount(GetNumIterations() * (GetNumIterations() - 1) / 2u * 100'000);
			for (const auto& sum : sums)
				EXPECT_EQ(expectedSum, sum);
		}
	}

	NO_STRESS_TEST(TEST_CLASS, CacheCommitIsThreadSafeWithSingleReaderSingleWriter) {
		RunMultithreadedReadWriteTest(1);
	}

	NO_STRESS_TEST(TEST_CLASS, CacheCommitIsThreadSafeWithMultipleReadersSingleWriter) {
		RunMultithreadedReadWriteTest(test::GetNumDefaultPoolThreads());
	}

	NO_STRESS_TEST(TEST_CLASS, CanAddManyAccounts) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto delta = cache.createDelta();

			// Act:
			test::StressThreadLogger logger("main thread");
			for (auto i = 0u; i < GetNumStressAccounts(); ++i) {
				logger.notifyIteration(i, GetNumStressAccounts());

				auto key = GetKeyFromId(i);
				delta->addAccount(key, Height(456));
				auto accountStateIter = delta->find(key);
				auto& accountState = accountStateIter.get();
				accountState.Balances.credit(Transferable_Mosaic_Id, Amount(i * 100'000));
				cache.commit();
			}
		}

		// Assert:
		EXPECT_EQ(GetNumStressAccounts(), cache.createView()->size());
	}

	// region hash cache performance

	namespace {
		using Generator = supplier<uint64_t>;
		using Samples = std::vector<state::TimestampedHash>;

		class Stopwatch final {
		public:
			Stopwatch(size_t numIterations, const std::string& message)
					: m_numIterations(numIterations)
					, m_message(message)
					, m_start(std::chrono::steady_clock::now())
			{}

			~Stopwatch() {
				CATAPULT_LOG(warning) << m_message << " needs " << nanos() / m_numIterations << "ns";
			}

		private:
			uint64_t nanos() const {
				auto elapsedDuration = std::chrono::steady_clock::now() - m_start;
				return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(elapsedDuration).count());
			}

		private:
			size_t m_numIterations;
			std::string m_message;
			std::chrono::steady_clock::time_point m_start;
		};

		Hash256 GenerateRandomHash(const Generator& generator) {
			Hash256 data;
			auto start = reinterpret_cast<uint64_t*>(data.data());
			start[0] = generator();
			start[1] = generator();
			start[2] = generator();
			start[3] = generator();
			return data;
		}

		Samples CreateSamples(size_t count, const Generator& generator) {
			Samples timestampedHashes;
			timestampedHashes.reserve(count);
			for (auto i = 0u; i < count; ++i)
				timestampedHashes.push_back(state::TimestampedHash(Timestamp(i), GenerateRandomHash(generator)));

			return timestampedHashes;
		}

		void PopulateCache(HashCache& cache, size_t count, const Generator& generator) {
			auto delta = cache.createDelta();
			for (auto i = 0u; i < count; ++i) {
				auto hash = GenerateRandomHash(generator);
				Timestamp timestamp(i);
				delta->insert(state::TimestampedHash(timestamp, hash));
				if (0 == (i + 1) % (count / 10))
					CATAPULT_LOG(info) << "hash cache size is " << delta->size();
			}

			cache.commit();
		}

		uint64_t InsertTest(const Samples& samples, size_t count, HashCache& cache) {
			auto delta = cache.createDelta();

			Stopwatch stopwatch(count, "insert value");
			uint64_t value = 0;
			for (auto i = 0u; i < count; ++i) {
				delta->insert(samples[i]);
				value += samples[i].Hash[0];
			}

			cache.commit();
			return value;
		}

		uint64_t ContainsTest(const Samples& samples, size_t count, const HashCache& cache) {
			auto view = cache.createView();

			Stopwatch stopwatch(count, "contains value");
			uint64_t value = 0;
			for (auto i = 0u; i < count; ++i) {
				auto isContained = view->contains(samples[i]);
				value += isContained ? 1 : 0;
			}

			return value;
		}

		int64_t RemoveTest(const Samples& samples, size_t count, HashCache& cache) {
			auto delta = cache.createDelta();

			Stopwatch stopwatch(count, "remove value");
			int64_t value = 0;
			for (auto i = 0u; i < count; ++i) {
				delta->remove(samples[i]);
				value -= samples[i].Hash[0];
			}

			cache.commit();
			return value;
		}
	}

	NO_STRESS_TEST(TEST_CLASS, HashCachePerformance) {
		// Arrange:
		// - initialCount: how many entities the cache should initially contain
		// - numOperations: how many operations are done for the test
		auto initialCount = test::GetStressIterationCount() ? 50'000'000u : 100'000u;
		auto numOperations = test::GetStressIterationCount() ? 20'000u : 100'000u;
		cache::HashCache cache(CacheConfiguration(), utils::TimeSpan::FromHours(1));

		auto samples = CreateSamples(numOperations, test::Random);
		PopulateCache(cache, initialCount, test::Random);

		// Act:
		auto value = InsertTest(samples, numOperations, cache);
		value += ContainsTest(samples, numOperations, cache);
		value += static_cast<uint64_t>(RemoveTest(samples, numOperations, cache));
		value += ContainsTest(samples, numOperations, cache);

		// Assert: insert and remove tests cancel each other with regards to value,
		//         first contains test adds 1 for each operation, second one leaves value unchanged
		EXPECT_EQ(numOperations, value);
	}

	NO_STRESS_TEST(TEST_CLASS, HashCachePruneTest) {
		// Arrange:
		auto multipler = test::GetStressIterationCount() ? 10u : 1u;
		auto entriesCount = 100'000 * multipler;
		auto pruneCount = 30'000 * multipler;
		test::TempDirectoryGuard dbDirGuard;
		CacheConfiguration config(dbDirGuard.name(), utils::FileSize::FromMegabytes(5), PatriciaTreeStorageMode::Disabled);

		// - set retention time to 0, to simplify test
		HashCache cache(config, utils::TimeSpan::FromSeconds(0));
		{
			auto delta = cache.createDelta();

			Stopwatch stopwatch(entriesCount, "rocks-based insert");
			for (auto i = 0u; i < entriesCount; ++i)
				delta->insert(state::TimestampedHash(Timestamp(1000 * i)));

			cache.commit();
		}

		// Sanity:
		EXPECT_EQ(entriesCount, cache.createView()->size());

		// Act: prune entries below specified value
		{
			auto delta = cache.createDelta();

			Stopwatch stopwatch(pruneCount, "rocks-based prune");
			delta->prune(Timestamp(1000 * pruneCount));
			cache.commit();
		}

		// Assert:
		auto view = cache.createView();
		EXPECT_EQ(entriesCount - pruneCount, view->size());

		Stopwatch stopwatch(entriesCount, "rocks-based contains");
		for (auto i = 0u; i < pruneCount; ++i)
			EXPECT_FALSE(view->contains(state::TimestampedHash(Timestamp(1000 * i))));

		for (auto i = pruneCount; i < entriesCount; ++i)
			EXPECT_TRUE(view->contains(state::TimestampedHash(Timestamp(1000 * i))));
	}

	// endregion

	// region account state cache performance

	namespace {
		using Addresses = std::vector<Address>;
		using Keys = std::vector<Key>;

		Address GenerateAddress(const Generator& generator) {
			Address data;
			auto start = reinterpret_cast<uint64_t*>(data.data());
			start[0] = generator();
			start[1] = generator();
			start[2] = generator();
			return data;
		}

		Key GenerateKey(const Generator& generator) {
			Key data;
			auto start = reinterpret_cast<uint64_t*>(data.data());
			start[0] = generator();
			start[1] = generator();
			start[2] = generator();
			start[3] = generator();
			return data;
		}

		Addresses CreateAddresses(size_t count, const Generator& generator) {
			std::vector<Address> addresses;
			addresses.reserve(count);
			for (auto i = 0u; i < count; ++i)
				addresses.push_back(GenerateAddress(generator));

			return addresses;
		}

		Keys CreateKeys(size_t count, const Generator& generator) {
			std::vector<Key> keys;
			keys.reserve(count);
			for (auto i = 0u; i < count; ++i)
				keys.push_back(GenerateKey(generator));

			return keys;
		}

		template<typename TEntities>
		uint64_t InsertAccounts(const TEntities& entities, size_t count, AccountStateCache& cache, const char* message) {
			auto delta = cache.createDelta();

			Stopwatch stopwatch(count, message);
			uint64_t value = 0;
			for (auto i = 0u; i < count; ++i) {
				delta->addAccount(entities[i], Height(456));
				value += entities[i][0];
			}

			cache.commit();
			return value;
		}

		uint64_t InsertAccounts(const Addresses& addresses, size_t count, AccountStateCache& cache) {
			return InsertAccounts(addresses, count, cache, "add account via address");
		}

		uint64_t InsertAccounts(const Keys& keys, size_t count, AccountStateCache& cache) {
			return InsertAccounts(keys, count, cache, "add account via public key");
		}
	}

	NO_STRESS_TEST(TEST_CLASS, AccountStateCachePerformance) {
		// Arrange:
		constexpr size_t Num_Operations = 100'000;
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);

		auto addresses = CreateAddresses(Num_Operations, test::Random);
		auto keys = CreateKeys(Num_Operations, test::Random);

		// Act:
		auto value = InsertAccounts(keys, Num_Operations, cache);
		value += InsertAccounts(addresses, Num_Operations, cache);

		// Assert:
		EXPECT_EQ(2 * Num_Operations, cache.createView()->size());
	}

	// endregion
}}

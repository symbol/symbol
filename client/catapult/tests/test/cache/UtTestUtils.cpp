#include "UtTestUtils.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/model/EntityRange.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	void AddAll(cache::UtCache& cache, const std::vector<model::TransactionInfo>& transactionInfos) {
		auto modifier = cache.modifier();
		for (const auto& transactionInfo : transactionInfos)
			modifier.add(transactionInfo);
	}

	void RemoveAll(cache::UtCache& cache, const std::vector<Hash256>& hashes) {
		auto modifier = cache.modifier();
		for (const auto& hash : hashes)
			modifier.remove(hash);
	}

	std::vector<Timestamp::ValueType> ExtractRawDeadlines(const cache::MemoryUtCache& cache) {
		std::vector<Timestamp::ValueType> rawDeadlines;

		auto view = cache.view();
		view.forEach([&rawDeadlines](const auto& info) {
			rawDeadlines.push_back(info.pEntity->Deadline.unwrap());
			return true;
		});
		return rawDeadlines;
	}

	std::vector<std::unique_ptr<const model::Transaction>> ExtractTransactions(const cache::MemoryUtCache& cache, size_t count) {
		std::vector<std::unique_ptr<const model::Transaction>> transactions;

		auto view = cache.view();
		if (0 != count) {
			view.forEach([count, &transactions](const auto& info) {
				transactions.push_back(test::CopyTransaction(*info.pEntity));
				return count != transactions.size();
			});
		}

		return transactions;
	}

	std::vector<const model::TransactionInfo*> ExtractTransactionInfos(const cache::MemoryUtCacheView& view, size_t count) {
		std::vector<const model::TransactionInfo*> infos;

		if (0 != count) {
			view.forEach([count, &infos](const auto& info) {
				infos.push_back(&info);
				return count != infos.size();
			});
		}

		return infos;
	}

	void AssertDeadlines(const cache::MemoryUtCache& cache, const std::vector<Timestamp::ValueType>& expectedDeadlines) {
		EXPECT_EQ(expectedDeadlines, ExtractRawDeadlines(cache));
	}

	namespace {
		void AssertContainsAll(const cache::MemoryUtCache& cache, const std::vector<Hash256>& hashes, bool shouldContain) {
			auto i = 0u;
			auto view = cache.view();
			for (const auto& hash : hashes) {
				if (shouldContain)
					EXPECT_TRUE(view.contains(hash)) << "hash at " << i;
				else
					EXPECT_FALSE(view.contains(hash)) << "hash at " << i;

				++i;
			}
		}
	}

	void AssertContainsAll(const cache::MemoryUtCache& cache, const std::vector<Hash256>& hashes) {
		AssertContainsAll(cache, hashes, true);
	}

	void AssertContainsNone(const cache::MemoryUtCache& cache, const std::vector<Hash256>& hashes) {
		AssertContainsAll(cache, hashes, false);
	}

	namespace {
		void AssertContainsAll(
				const cache::MemoryUtCache& cache,
				const std::vector<model::TransactionInfo>& transactionInfos,
				bool shouldContain) {
			auto i = 0u;
			auto view = cache.view();
			for (const auto& transactionInfo : transactionInfos) {
				if (shouldContain)
					EXPECT_TRUE(view.contains(transactionInfo.EntityHash)) << "info at " << i;
				else
					EXPECT_FALSE(view.contains(transactionInfo.EntityHash)) << "info at " << i;

				++i;
			}
		}
	}

	void AssertContainsAll(const cache::MemoryUtCache& cache, const std::vector<model::TransactionInfo>& transactionInfos) {
		AssertContainsAll(cache, transactionInfos, true);
	}

	void AssertContainsNone(const cache::MemoryUtCache& cache, const std::vector<model::TransactionInfo>& transactionInfos) {
		AssertContainsAll(cache, transactionInfos, false);
	}
}}

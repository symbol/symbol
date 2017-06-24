#include "UnconfirmedTransactionsTestUtils.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/model/EntityRange.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	model::TransactionInfo CreateTransactionInfoWithDeadline(size_t deadline) {
		auto pTransaction = test::GenerateRandomTransaction();
		pTransaction->Deadline = Timestamp(deadline);
		return model::TransactionInfo(
				std::move(pTransaction),
				test::GenerateRandomData<Hash256_Size>(),
				test::GenerateRandomData<Hash256_Size>());
	}

	std::vector<model::TransactionInfo> CreateTransactionInfos(
			size_t count,
			const std::function<Timestamp (size_t)>& deadlineGenerator) {
		std::vector<model::TransactionInfo> infos;
		for (auto i = 0u; i < count; ++i)
			infos.push_back(CreateTransactionInfoWithDeadline(deadlineGenerator(i).unwrap()));

		return infos;
	}

	std::vector<model::TransactionInfo> CreateTransactionInfos(size_t count) {
		return CreateTransactionInfos(count, [](auto i) { return Timestamp(i + 1); });
	}

	std::vector<const model::VerifiableEntity*> ExtractEntities(const std::vector<model::TransactionInfo>& infos) {
		std::vector<const model::VerifiableEntity*> entities;
		for (const auto& info : infos)
			entities.push_back(info.pEntity.get());

		return entities;
	}

	std::vector<Hash256> ExtractHashes(const std::vector<model::TransactionInfo>& infos) {
		std::vector<Hash256> hashes;
		for (const auto& info : infos)
			hashes.push_back(info.EntityHash);

		return hashes;
	}

	void AddAll(cache::UtCache& cache, std::vector<model::TransactionInfo>&& infos) {
		auto modifier = cache.modifier();
		for (auto& info : infos)
			modifier.add(std::move(info));
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

	model::EntityRange<utils::ShortHash> ExtractShortHashes(const cache::MemoryUtCache& cache) {
		auto view = cache.view();
		auto shortHashes = model::EntityRange<utils::ShortHash>::PrepareFixed(view.size());
		auto shortHashesIter = shortHashes.begin();

		view.forEach([&shortHashesIter](const auto& info) {
			*shortHashesIter++ = *reinterpret_cast<const utils::ShortHash*>(info.EntityHash.data());
			return true;
		});
		return shortHashes;
	}

	void AssertDeadlines(
			const cache::MemoryUtCache& cache,
			const std::vector<Timestamp::ValueType>& expectedDeadlines) {
		EXPECT_EQ(expectedDeadlines, ExtractRawDeadlines(cache));
	}

	namespace {
		void AssertContainsAll(
				const cache::MemoryUtCache& cache,
				const std::vector<Hash256>& hashes,
				bool shouldContain) {
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

	void AssertContainsAll(
			const cache::MemoryUtCache& cache,
			const std::vector<Hash256>& hashes) {
		AssertContainsAll(cache, hashes, true);
	}

	void AssertContainsNone(
			const cache::MemoryUtCache& cache,
			const std::vector<Hash256>& hashes) {
		AssertContainsAll(cache, hashes, false);
	}

	namespace {
		void AssertContainsAll(
				const cache::MemoryUtCache& cache,
				const std::vector<model::TransactionInfo>& infos,
				bool shouldContain) {
			auto i = 0u;
			auto view = cache.view();
			for (const auto& info : infos) {
				if (shouldContain)
					EXPECT_TRUE(view.contains(info.EntityHash)) << "info at " << i;
				else
					EXPECT_FALSE(view.contains(info.EntityHash)) << "info at " << i;

				++i;
			}
		}
	}

	void AssertContainsAll(
			const cache::MemoryUtCache& cache,
			const std::vector<model::TransactionInfo>& infos) {
		AssertContainsAll(cache, infos, true);
	}

	void AssertContainsNone(
			const cache::MemoryUtCache& cache,
			const std::vector<model::TransactionInfo>& infos) {
		AssertContainsAll(cache, infos, false);
	}

	void AssertEqual(const model::TransactionInfo& lhs, const model::TransactionInfo& rhs) {
		EXPECT_EQ(*lhs.pEntity, *rhs.pEntity);
		EXPECT_EQ(lhs.EntityHash, rhs.EntityHash);
		EXPECT_EQ(lhs.MerkleComponentHash, rhs.MerkleComponentHash);
	}
}}

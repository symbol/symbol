#include "TransactionInfoTestUtils.h"
#include "TransactionTestUtils.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	model::TransactionInfo CreateRandomTransactionInfo() {
		auto transactionInfo = model::TransactionInfo(test::GenerateRandomTransaction());
		test::FillWithRandomData(transactionInfo.EntityHash);
		test::FillWithRandomData(transactionInfo.MerkleComponentHash);
		transactionInfo.OptionalExtractedAddresses = std::make_shared<model::AddressSet>();
		return transactionInfo;
	}

	model::TransactionInfo CreateTransactionInfoWithDeadline(size_t deadline) {
		auto pTransaction = test::GenerateRandomTransaction();
		pTransaction->Deadline = Timestamp(deadline);
		auto transactionInfo = model::TransactionInfo(std::move(pTransaction));
		test::FillWithRandomData(transactionInfo.EntityHash);
		test::FillWithRandomData(transactionInfo.MerkleComponentHash);
		transactionInfo.OptionalExtractedAddresses = std::make_shared<model::AddressSet>();
		return transactionInfo;
	}

	std::vector<model::TransactionInfo> CreateTransactionInfos(size_t count, const std::function<Timestamp (size_t)>& deadlineGenerator) {
		std::vector<model::TransactionInfo> transactionInfos;
		for (auto i = 0u; i < count; ++i)
			transactionInfos.push_back(CreateTransactionInfoWithDeadline(deadlineGenerator(i).unwrap()));

		return transactionInfos;
	}

	std::vector<model::TransactionInfo> CreateTransactionInfos(size_t count) {
		return CreateTransactionInfos(count, [](auto i) { return Timestamp(i + 1); });
	}

	std::vector<model::TransactionInfo> CopyTransactionInfos(const std::vector<model::TransactionInfo>& transactionInfos) {
		std::vector<model::TransactionInfo> copy;
		for (const auto& transactionInfo : transactionInfos)
			copy.emplace_back(transactionInfo.copy());

		return copy;
	}

	model::TransactionInfosSet CopyTransactionInfosToSet(const std::vector<model::TransactionInfo>& transactionInfos) {
		model::TransactionInfosSet copy;
		for (const auto& transactionInfo : transactionInfos)
			copy.emplace(transactionInfo.copy());

		return copy;
	}

	std::vector<const model::VerifiableEntity*> ExtractEntities(const std::vector<model::TransactionInfo>& transactionInfos) {
		std::vector<const model::VerifiableEntity*> entities;
		for (const auto& transactionInfo : transactionInfos)
			entities.push_back(transactionInfo.pEntity.get());

		return entities;
	}

	std::vector<Hash256> ExtractHashes(const std::vector<model::TransactionInfo>& transactionInfos) {
		std::vector<Hash256> hashes;
		for (const auto& transactionInfo : transactionInfos)
			hashes.push_back(transactionInfo.EntityHash);

		return hashes;
	}

	void AssertEqual(const model::DetachedTransactionInfo& lhs, const model::DetachedTransactionInfo& rhs, const std::string& message) {
		EXPECT_EQ(*lhs.pEntity, *rhs.pEntity) << message;
		EXPECT_EQ(test::ToString(lhs.EntityHash), test::ToString(rhs.EntityHash)) << message;
		EXPECT_EQ(lhs.OptionalExtractedAddresses.get(), rhs.OptionalExtractedAddresses.get()) << message;
	}

	void AssertEqual(const model::TransactionInfo& lhs, const model::TransactionInfo& rhs, const std::string& message) {
		EXPECT_EQ(*lhs.pEntity, *rhs.pEntity) << message;
		EXPECT_EQ(test::ToString(lhs.EntityHash), test::ToString(rhs.EntityHash)) << message;
		EXPECT_EQ(lhs.OptionalExtractedAddresses.get(), rhs.OptionalExtractedAddresses.get()) << message;
		EXPECT_EQ(test::ToString(lhs.MerkleComponentHash), test::ToString(rhs.MerkleComponentHash)) << message;
	}

	namespace {
		std::map<Hash256, model::TransactionInfo> ToMap(const std::vector<model::TransactionInfo>& transactionInfos) {
			std::map<Hash256, model::TransactionInfo> map;
			for (const auto& transactionInfo : transactionInfos)
				map.emplace(transactionInfo.EntityHash, transactionInfo.copy());

			return map;
		}
	}

	void AssertEquivalent(
			const std::vector<model::TransactionInfo>& lhs,
			const std::vector<model::TransactionInfo>& rhs,
			const std::string& message) {
		ASSERT_EQ(lhs.size(), rhs.size()) << message;

		auto lhsMap = ToMap(lhs);
		auto rhsMap = ToMap(rhs);

		auto lhsIter = lhsMap.cbegin();
		auto rhsIter = rhsMap.cbegin();
		for (auto i = 0u; i < lhs.size(); ++i) {
			std::ostringstream out;
			out << message << " " << i << " (" << utils::HexFormat(lhsIter->first) << ")";
			AssertEqual(lhsIter->second, rhsIter->second, out.str());

			++lhsIter;
			++rhsIter;
		}
	}
}}

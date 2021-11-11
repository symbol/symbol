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

#include "TransactionInfoTestUtils.h"
#include "TransactionTestUtils.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region create (single)

	namespace {
		void Prepare(model::TransactionInfo& transactionInfo) {
			FillWithRandomData(transactionInfo.EntityHash);
			FillWithRandomData(transactionInfo.MerkleComponentHash);
			transactionInfo.OptionalExtractedAddresses = std::make_shared<model::UnresolvedAddressSet>();
		}
	}

	model::TransactionInfo CreateRandomTransactionInfo() {
		auto transactionInfo = model::TransactionInfo(GenerateRandomTransaction());
		Prepare(transactionInfo);
		return transactionInfo;
	}

	model::TransactionInfo CreateRandomTransactionInfoWithSize(uint32_t entitySize) {
		auto transactionInfo = model::TransactionInfo(GenerateRandomTransactionWithSize(entitySize));
		Prepare(transactionInfo);
		return transactionInfo;
	}

	model::TransactionInfo CreateTransactionInfoWithDeadline(uint64_t deadline) {
		auto pTransaction = GenerateRandomTransaction();
		pTransaction->Deadline = Timestamp(deadline);
		auto transactionInfo = model::TransactionInfo(std::move(pTransaction));
		Prepare(transactionInfo);
		return transactionInfo;
	}

	// endregion

	// region create (multiple)

	std::vector<model::TransactionInfo> CreateTransactionInfos(size_t count) {
		return CreateTransactionInfos(count, [](auto i) { return Timestamp(i + 1); });
	}

	std::vector<model::TransactionInfo> CreateTransactionInfos(size_t count, const std::function<Timestamp (size_t)>& deadlineGenerator) {
		std::vector<model::TransactionInfo> transactionInfos;
		for (auto i = 0u; i < count; ++i)
			transactionInfos.push_back(CreateTransactionInfoWithDeadline(deadlineGenerator(i).unwrap()));

		return transactionInfos;
	}

	std::vector<model::TransactionInfo> CreateTransactionInfosFromSizeMultiplierPairs(
			const std::vector<std::pair<uint32_t, uint32_t>>& sizeMultiplierPairs) {
		std::vector<model::TransactionInfo> transactionInfos;
		for (const auto& pair: sizeMultiplierPairs) {
			auto pTransaction = GenerateRandomTransactionWithSize(pair.first);
			pTransaction->MaxFee = Amount(pair.first * pair.second / 10);
			transactionInfos.push_back(model::TransactionInfo(std::move(pTransaction)));

			// give each info a unique entity hash so that they can all be added to a UT cache
			FillWithRandomData(transactionInfos.back().EntityHash);
		}

		return transactionInfos;
	}

	std::vector<model::TransactionInfo> CreateTransactionInfosWithOptionalAddresses(size_t count) {
		std::vector<model::TransactionInfo> transactionInfos;
		for (auto i = 0u; i < count; ++i) {
			auto transactionInfo = test::CreateRandomTransactionInfo();
			transactionInfo.OptionalExtractedAddresses = test::GenerateRandomUnresolvedAddressSetPointer(i % 2 + 1);
			transactionInfos.push_back(std::move(transactionInfo));
		}

		return transactionInfos;
	}

	// endregion

	// region copy

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

	// endregion

	// region extract

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

	// endregion

	// region asserts

	void AssertEqual(const model::DetachedTransactionInfo& lhs, const model::DetachedTransactionInfo& rhs, const std::string& message) {
		EXPECT_EQ(*lhs.pEntity, *rhs.pEntity) << message;
		EXPECT_EQ(lhs.EntityHash, rhs.EntityHash) << message;
		EXPECT_EQ(lhs.OptionalExtractedAddresses.get(), rhs.OptionalExtractedAddresses.get()) << message;
	}

	void AssertEqual(const model::TransactionInfo& lhs, const model::TransactionInfo& rhs, const std::string& message) {
		EXPECT_EQ(*lhs.pEntity, *rhs.pEntity) << message;
		EXPECT_EQ(lhs.EntityHash, rhs.EntityHash) << message;
		if (lhs.OptionalExtractedAddresses && rhs.OptionalExtractedAddresses)
			EXPECT_EQ(*lhs.OptionalExtractedAddresses, *rhs.OptionalExtractedAddresses) << message;
		else
			EXPECT_EQ(lhs.OptionalExtractedAddresses.get(), rhs.OptionalExtractedAddresses.get()) << message;

		EXPECT_EQ(lhs.MerkleComponentHash, rhs.MerkleComponentHash) << message;
	}

	namespace {
		template<typename TContainer>
		std::map<Hash256, model::TransactionInfo> ToMap(const TContainer& transactionInfos) {
			std::map<Hash256, model::TransactionInfo> map;
			for (const auto& transactionInfo : transactionInfos)
				map.emplace(transactionInfo.EntityHash, transactionInfo.copy());

			return map;
		}

		template<typename TContainer>
		void AssertEquivalentImpl(const TContainer& lhs, const TContainer& rhs, const std::string& message) {
			ASSERT_EQ(lhs.size(), rhs.size()) << message;

			auto lhsMap = ToMap(lhs);
			auto rhsMap = ToMap(rhs);

			auto lhsIter = lhsMap.cbegin();
			auto rhsIter = rhsMap.cbegin();
			for (auto i = 0u; i < lhs.size(); ++i) {
				std::ostringstream out;
				out << message << " " << i << " (" << lhsIter->first << ")";
				AssertEqual(lhsIter->second, rhsIter->second, out.str());

				++lhsIter;
				++rhsIter;
			}
		}
	}

	void AssertEquivalent(
			const std::vector<model::TransactionInfo>& lhs,
			const std::vector<model::TransactionInfo>& rhs,
			const std::string& message) {
		AssertEquivalentImpl(lhs, rhs, message);
	}

	void AssertEquivalent(const model::TransactionInfosSet& lhs, const model::TransactionInfosSet& rhs, const std::string& message) {
		AssertEquivalentImpl(lhs, rhs, message);
	}

	// endregion
}}

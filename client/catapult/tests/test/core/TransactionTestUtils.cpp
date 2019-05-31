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

#include "TransactionTestUtils.h"
#include "EntityTestUtils.h"
#include "sdk/src/extensions/TransactionExtensions.h"
#include "mocks/MockTransaction.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/TestConstants.h"
#include <memory>

namespace catapult { namespace test {

	GenerationHash GetDefaultGenerationHash() {
		return utils::ParseByteArray<GenerationHash>("AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDD");
	}

	std::unique_ptr<model::Transaction> GenerateRandomTransaction() {
		return GenerateRandomTransaction(GetDefaultGenerationHash());
	}

	std::unique_ptr<model::Transaction> GenerateRandomTransaction(const GenerationHash& generationHash) {
		auto signer = GenerateKeyPair();
		auto pTransaction = GenerateRandomTransaction(signer.publicKey());
		extensions::TransactionExtensions(generationHash).sign(signer, *pTransaction);
		return pTransaction;
	}

	std::unique_ptr<model::Transaction> GenerateRandomTransaction(const Key& signer) {
		auto pTransaction = mocks::CreateMockTransaction(12);
		pTransaction->Signer = signer;
		pTransaction->Version = model::MakeVersion(model::NetworkIdentifier::Mijin_Test, 1);
		return std::move(pTransaction);
	}

	MutableTransactions GenerateRandomTransactions(size_t count) {
		std::vector<std::shared_ptr<model::Transaction>> transactions;
		for (auto i = 0u; i < count; ++i)
			transactions.push_back(GenerateRandomTransaction());

		return transactions;
	}

	ConstTransactions MakeConst(const MutableTransactions& transactions) {
		ConstTransactions constTransactions;
		for (const auto& pTransaction : transactions)
			constTransactions.push_back(pTransaction);

		return constTransactions;
	}

	std::unique_ptr<model::Transaction> GenerateRandomTransactionWithSize(size_t entitySize) {
		auto pEntity = utils::MakeUniqueWithSize<model::Transaction>(entitySize);
		FillWithRandomData(MutableRawBuffer{ reinterpret_cast<uint8_t*>(pEntity.get()), entitySize });
		pEntity->Size = static_cast<uint32_t>(entitySize);
		pEntity->Type = static_cast<model::EntityType>(0x4000 | (0x0FFF & utils::to_underlying_type(pEntity->Type)));
		return pEntity;
	}

	std::unique_ptr<model::Transaction> GenerateTransactionWithDeadline(Timestamp deadline) {
		auto pTransaction = GenerateRandomTransaction();
		pTransaction->Deadline = deadline;
		return pTransaction;
	}

	std::unique_ptr<model::Transaction> GenerateDeterministicTransaction() {
		auto keyPair = crypto::KeyPair::FromString("4C730B716552D60D276D24EABD60CAB3BF15BC6824937A510639CEB08BC77821");

		auto pTransaction = mocks::CreateMockTransaction(sizeof(uint64_t));
		pTransaction->Signer = keyPair.publicKey();
		pTransaction->Version = 1;
		pTransaction->MaxFee = Amount(2468);
		pTransaction->Deadline = Timestamp(45678);
		pTransaction->Recipient = crypto::ParseKey("72B69A64B20AF34C3815073647C8A2354800E8E83B718303909ABDC0F38E7ED7");
		reinterpret_cast<uint64_t&>(*pTransaction->DataPtr()) = 12345;

		auto generationHash = utils::ParseByteArray<GenerationHash>(test::Deterministic_Network_Generation_Hash_String);
		extensions::TransactionExtensions(generationHash).sign(keyPair, *pTransaction);
		return std::move(pTransaction);
	}

	namespace {
		std::vector<uint8_t> CreateRandomTransactionBuffer(size_t numTransactions) {
			constexpr auto Entity_Size = sizeof(model::Transaction);
			auto buffer = GenerateRandomVector(numTransactions * Entity_Size);
			for (auto i = 0u; i < numTransactions; ++i) {
				auto& transaction = reinterpret_cast<model::Transaction&>(buffer[i * Entity_Size]);
				transaction.Size = Entity_Size;
				transaction.Type = mocks::MockTransaction::Entity_Type;
			}

			return buffer;
		}
	}

	model::TransactionRange CreateTransactionEntityRange(size_t numTransactions) {
		auto buffer = CreateRandomTransactionBuffer(numTransactions);
		return model::TransactionRange::CopyFixed(buffer.data(), numTransactions);
	}

	model::TransactionRange CreateEntityRange(const std::vector<const model::Transaction*>& transactions) {
		return CreateEntityRange<model::Transaction>(transactions);
	}

	model::DetachedCosignature CreateRandomCosignature() {
		return model::DetachedCosignature{
			test::GenerateRandomByteArray<Key>(),
			test::GenerateRandomByteArray<Signature>(),
			test::GenerateRandomByteArray<Hash256>(),
		};
	}
}}

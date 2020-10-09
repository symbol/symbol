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
#include "catapult/crypto/Signer.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/utils/HexParser.h"
#include "catapult/preprocessor.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/TestConstants.h"
#include <memory>

namespace catapult { namespace test {

	GenerationHashSeed GetDefaultGenerationHashSeed() {
		return utils::ParseByteArray<GenerationHashSeed>("AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDD");
	}

	std::unique_ptr<model::Transaction> GenerateRandomTransaction() {
		return GenerateRandomTransaction(GetDefaultGenerationHashSeed());
	}

	std::unique_ptr<model::Transaction> GenerateRandomTransaction(const GenerationHashSeed& generationHashSeed) {
		auto signer = GenerateKeyPair();
		auto pTransaction = GenerateRandomTransaction(signer.publicKey());
		extensions::TransactionExtensions(generationHashSeed).sign(signer, *pTransaction);
		return pTransaction;
	}

	std::unique_ptr<model::Transaction> GenerateRandomTransaction(const Key& signer) {
		auto pTransaction = mocks::CreateMockTransaction(12);
		pTransaction->SignerPublicKey = signer;
		pTransaction->Version = 1;
		pTransaction->Network = model::NetworkIdentifier::Private_Test;
		return PORTABLE_MOVE(pTransaction);
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
		pTransaction->SignerPublicKey = keyPair.publicKey();
		pTransaction->Version = 1;
		pTransaction->Network = model::NetworkIdentifier::Zero;
		pTransaction->MaxFee = Amount(2468);
		pTransaction->Deadline = Timestamp(45678);
		pTransaction->RecipientPublicKey = utils::ParseByteArray<Key>("72B69A64B20AF34C3815073647C8A2354800E8E83B718303909ABDC0F38E7ED7");
		reinterpret_cast<uint64_t&>(*pTransaction->DataPtr()) = 12345;

		auto generationHashSeed = utils::ParseByteArray<GenerationHashSeed>(test::Deterministic_Network_Generation_Hash_Seed_String);
		extensions::TransactionExtensions(generationHashSeed).sign(keyPair, *pTransaction);
		return PORTABLE_MOVE(pTransaction);
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

	model::DetachedCosignature CreateRandomDetachedCosignature() {
		auto cosignature = model::DetachedCosignature(
				test::GenerateRandomByteArray<Key>(),
				test::GenerateRandomByteArray<Signature>(),
				test::GenerateRandomByteArray<Hash256>());
		cosignature.Version = Random();
		return cosignature;
	}

	void AssertCosignature(
			const model::Cosignature& expectedCosignature,
			const model::Cosignature& actualCosignature,
			const std::string& message) {
		EXPECT_EQ(expectedCosignature.Version, actualCosignature.Version) << message;
		EXPECT_EQ(expectedCosignature.SignerPublicKey, actualCosignature.SignerPublicKey) << message;
		EXPECT_EQ(expectedCosignature.Signature, actualCosignature.Signature) << message;
	}

	void AssertCosignatures(
			const std::vector<model::Cosignature>& expectedCosignatures,
			const std::vector<model::Cosignature>& actualCosignatures,
			const std::string& message) {
		ASSERT_EQ(expectedCosignatures.size(), actualCosignatures.size()) << message;

		for (auto i = 0u; i < expectedCosignatures.size(); ++i)
			AssertCosignature(expectedCosignatures[i], actualCosignatures[i], message + ", cosignature at " + std::to_string(i));
	}
}}

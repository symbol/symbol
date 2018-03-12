#include "TransactionTestUtils.h"
#include "EntityTestUtils.h"
#include "sdk/src/extensions/TransactionExtensions.h"
#include "mocks/MockTransaction.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/VerifiableEntity.h"
#include <memory>

namespace catapult { namespace test {

	std::unique_ptr<model::Transaction> GenerateRandomTransaction() {
		auto signer = GenerateKeyPair();
		auto pTransaction = mocks::CreateMockTransaction(12);
		pTransaction->Signer = signer.publicKey();
		pTransaction->Version = model::MakeVersion(model::NetworkIdentifier::Mijin_Test, 1);
		extensions::SignTransaction(signer, *pTransaction);
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

	std::unique_ptr<model::Transaction> GenerateRandomTransaction(size_t entitySize) {
		auto pEntity = utils::MakeUniqueWithSize<model::Transaction>(entitySize);
		FillWithRandomData(MutableRawBuffer{ reinterpret_cast<uint8_t*>(pEntity.get()), entitySize });
		pEntity->Size = static_cast<uint32_t>(entitySize);
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
		pTransaction->Fee = Amount(2468);
		pTransaction->Deadline = Timestamp(45678);
		pTransaction->Recipient = crypto::ParseKey("72B69A64B20AF34C3815073647C8A2354800E8E83B718303909ABDC0F38E7ED7");
		reinterpret_cast<uint64_t&>(*pTransaction->DataPtr()) = 12345;

		extensions::SignTransaction(keyPair, *pTransaction);
		return std::move(pTransaction);
	}

	std::unique_ptr<model::Transaction> CopyTransaction(const model::Transaction& transaction) {
		return CopyEntity(transaction);
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
			test::GenerateRandomData<Key_Size>(),
			test::GenerateRandomData<Signature_Size>(),
			test::GenerateRandomData<Hash256_Size>(),
		};
	}
}}

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

#include "src/builders/AggregateTransactionBuilder.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/EntityHasher.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/KeyTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS AggregateTransactionBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::AggregateTransaction>;

		Hash256 CalculateAggregateTransactionsHash(const std::vector<std::unique_ptr<mocks::EmbeddedMockTransaction>>& transactions) {
			crypto::MerkleHashBuilder transactionsHashBuilder(transactions.size());
			for (const auto& pTransaction : transactions) {
				Hash256 transactionHash;
				crypto::Sha3_256({ reinterpret_cast<const uint8_t*>(pTransaction.get()), pTransaction->Size }, transactionHash);
				transactionsHashBuilder.update(transactionHash);
			}

			Hash256 transactionsHash;
			transactionsHashBuilder.final(transactionsHash);
			return transactionsHash;
		}

		class TestContext {
		public:
			explicit TestContext(size_t numTransactions)
					: m_networkIdentifier(static_cast<model::NetworkIdentifier>(0x62))
					, m_signer(test::GenerateRandomByteArray<Key>())
					, m_builder(m_networkIdentifier, m_signer) {
				for (auto i = 0u; i < numTransactions; ++i)
					m_transactions.push_back(mocks::CreateEmbeddedMockTransaction(static_cast<uint16_t>(31 + i)));
			}

		public:
			auto buildTransaction() {
				for (const auto& pTransaction : m_transactions)
					m_builder.addTransaction(test::CopyEntity(*pTransaction));

				return m_builder.build();
			}

			void assertTransaction(const model::AggregateTransaction& transaction, size_t numCosignatures, model::EntityType type) {
				auto numTransactions = m_transactions.size();
				size_t payloadSize = numTransactions * sizeof(mocks::EmbeddedMockTransaction);
				for (auto i = 0u; i < numTransactions; ++i)
					payloadSize += 31 + i + utils::GetPaddingSize(sizeof(mocks::EmbeddedMockTransaction) + 31 + i, 8);

				// aggregate builder does not include cosignatures in size()
				RegularTraits::CheckBuilderSize(payloadSize, m_builder);

				auto additionalSize = payloadSize + numCosignatures * sizeof(model::Cosignature);
				RegularTraits::CheckFields(additionalSize, transaction);
				EXPECT_EQ(m_signer, transaction.SignerPublicKey);
				EXPECT_EQ(1u, transaction.Version);
				EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), transaction.Network);
				EXPECT_EQ(type, transaction.Type);

				EXPECT_EQ(CalculateAggregateTransactionsHash(m_transactions), transaction.TransactionsHash);
				ASSERT_EQ(payloadSize, transaction.PayloadSize);

				auto i = 0u;
				for (const auto& embedded : transaction.Transactions()) {
					ASSERT_EQ(m_transactions[i]->Size, embedded.Size) << "invalid size of transaction " << i;
					EXPECT_EQ_MEMORY(m_transactions[i].get(), &embedded, embedded.Size) << "invalid transaction " << i;
					++i;
				}

				// check padding bytes
				i = 0;
				auto transactionsSize = 0u;
				std::vector<uint8_t> transactionPaddingBytes;
				const auto* pTransactionBytes = reinterpret_cast<const uint8_t*>(transaction.TransactionsPtr());
				for (const auto& embedded : transaction.Transactions()) {
					pTransactionBytes += embedded.Size;
					transactionsSize += embedded.Size;

					auto paddingSize = utils::GetPaddingSize(embedded.Size, 8);
					transactionPaddingBytes.insert(transactionPaddingBytes.end(), pTransactionBytes, pTransactionBytes + paddingSize);
					pTransactionBytes += paddingSize;
				}

				EXPECT_EQ(payloadSize, transactionsSize + transactionPaddingBytes.size());
				EXPECT_EQ(std::vector<uint8_t>(transactionPaddingBytes.size(), 0), transactionPaddingBytes);
			}

		private:
			const model::NetworkIdentifier m_networkIdentifier;
			const Key m_signer;
			AggregateTransactionBuilder m_builder;
			std::vector<std::unique_ptr<mocks::EmbeddedMockTransaction>> m_transactions;
		};

		void AssertAggregateBuilderTransaction(size_t numTransactions) {
			// Arrange:
			TestContext context(numTransactions);

			// Act:
			auto pTransaction = context.buildTransaction();

			// Assert:
			context.assertTransaction(*pTransaction, 0, model::Entity_Type_Aggregate_Bonded);
		}

		auto GenerateKeys(size_t numKeys) {
			std::vector<crypto::KeyPair> keys;
			for (auto i = 0u; i < numKeys; ++i)
				keys.push_back(test::GenerateKeyPair());

			return keys;
		}

		RawBuffer TransactionDataBuffer(const model::AggregateTransaction& transaction) {
			return {
				reinterpret_cast<const uint8_t*>(&transaction) + model::Transaction::Header_Size,
				sizeof(model::AggregateTransaction) - model::Transaction::Header_Size - model::AggregateTransaction::Footer_Size
			};
		}

		void AssertAggregateCosignaturesTransaction(size_t numCosignatures) {
			// Arrange: create transaction with 3 embedded transactions
			TestContext context(3);
			auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
			AggregateCosignatureAppender builder(generationHashSeed, context.buildTransaction());
			auto cosignatories = GenerateKeys(numCosignatures);

			// Act:
			for (const auto& cosignatory : cosignatories)
				builder.cosign(cosignatory);

			auto pTransaction = builder.build();

			// Assert:
			context.assertTransaction(*pTransaction, cosignatories.size(), model::Entity_Type_Aggregate_Complete);
			auto hash = model::CalculateHash(*pTransaction, generationHashSeed, TransactionDataBuffer(*pTransaction));
			const auto* pCosignature = pTransaction->CosignaturesPtr();
			for (const auto& cosignatory : cosignatories) {
				EXPECT_EQ(cosignatory.publicKey(), pCosignature->SignerPublicKey) << "invalid signer";
				EXPECT_TRUE(crypto::Verify(pCosignature->SignerPublicKey, hash, pCosignature->Signature))
						<< "invalid cosignature " << pCosignature->Signature;
				++pCosignature;
			}
		}
	}

	TEST(TEST_CLASS, AggregateBuilderCreatesProperTransaction_SingleTransaction) {
		AssertAggregateBuilderTransaction(1);
	}

	TEST(TEST_CLASS, AggregateBuilderCreatesProperTransaction_MultipleTransactions) {
		AssertAggregateBuilderTransaction(3);
	}

	TEST(TEST_CLASS, AggregateCosignatureAppenderAddsProperSignature_SingleCosignatory) {
		AssertAggregateCosignaturesTransaction(1);
	}

	TEST(TEST_CLASS, AggregateCosignatureAppenderAddsProperSignature_MultipleCosignatories) {
		AssertAggregateCosignaturesTransaction(3);
	}
}}

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

#include "mongo/src/MongoTransactionStorage.h"
#include "mongo/tests/test/MongoTransactionStorageTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

	namespace {
		constexpr auto Ut_Collection_Name = "unconfirmedTransactions";

		std::vector<model::TransactionInfo> CreateTransactionInfos(
				size_t count,
				const std::function<Timestamp (size_t)>& deadlineGenerator) {
			std::vector<model::TransactionInfo> transactionInfos;
			for (auto i = 0u; i < count; ++i) {
				auto pTransaction = mocks::CreateMockTransaction(10);
				pTransaction->Deadline = deadlineGenerator(i);

				auto transactionInfo = test::CreateRandomTransactionInfo();
				transactionInfo.pEntity = std::move(pTransaction);
				transactionInfos.push_back(std::move(transactionInfo));
			}

			return transactionInfos;
		}
	}

	namespace {
		auto DeadlineGenerator(size_t i) {
			return Timestamp(10 * (i + 1));
		}

		class TransactionStorageContext {
		public:
			explicit TransactionStorageContext(
					size_t numTransactionInfos,
					test::DbInitializationType dbInitializationType = test::DbInitializationType::Reset)
					: TransactionStorageContext(numTransactionInfos, mocks::CreateMockTransactionMongoPlugin(), dbInitializationType)
			{}

			TransactionStorageContext(
					size_t numTransactionInfos,
					std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin,
					test::DbInitializationType dbInitializationType = test::DbInitializationType::Reset)
					: m_pStorage(test::CreateMongoStorage<cache::UtChangeSubscriber>(
							std::move(pTransactionPlugin),
							dbInitializationType,
							MongoErrorPolicy::Mode::Strict,
							[](auto& context, const auto& registry) {
								return CreateMongoTransactionStorage(context, registry, Ut_Collection_Name);
							}))
					, m_transactionInfos(CreateTransactionInfos(numTransactionInfos, DeadlineGenerator))
			{}

		public:
			std::string collectionName() {
				return Ut_Collection_Name;
			}

			std::vector<model::TransactionInfo>& transactionInfos() {
				return m_transactionInfos;
			}

			void saveTransaction(const model::TransactionInfo& transactionInfo) {
				cache::UtChangeSubscriber::TransactionInfos transactionInfos;
				transactionInfos.emplace(transactionInfo.copy());
				saveTransactions(transactionInfos);
			}

			void saveTransactions(const cache::UtChangeSubscriber::TransactionInfos& transactionInfos) {
				m_pStorage->notifyAdds(transactionInfos);
			}

			void removeTransaction(const model::TransactionInfo& transactionInfo) {
				cache::UtChangeSubscriber::TransactionInfos transactionInfos;
				transactionInfos.emplace(transactionInfo.copy());
				removeTransactions(transactionInfos);
			}

			void removeTransactions(const cache::UtChangeSubscriber::TransactionInfos& transactionInfos) {
				m_pStorage->notifyRemoves(transactionInfos);
			}

			void flush() {
				m_pStorage->flush();
			}

			void seedDatabase() {
				for (const auto& transactionInfo : m_transactionInfos)
					saveTransaction(transactionInfo);

				// Sanity:
				test::AssertCollectionSize(collectionName(), m_transactionInfos.size());
			}

		private:
			std::shared_ptr<cache::UtChangeSubscriber> m_pStorage;
			std::vector<model::TransactionInfo> m_transactionInfos;
		};
	}

	// region basic storage tests

	DEFINE_MONGO_TRANSACTION_STORAGE_SAVE_TESTS(TransactionStorage)
	DEFINE_MONGO_TRANSACTION_STORAGE_REMOVE_TESTS(TransactionStorage)
	DEFINE_MONGO_TRANSACTION_STORAGE_FLUSH_TESTS(TransactionStorage)

	// endregion
}}

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

#include "AdditionalTransactions.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include "catapult/utils/MemoryUtils.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		auto GetSortedPathNames(const std::string& directoryPath) {
			std::vector<std::string> pathNames;

			if (directoryPath.empty())
				return pathNames;

			auto begin = boost::filesystem::directory_iterator(directoryPath);
			auto end = boost::filesystem::directory_iterator();
			for (auto iter = begin; end != iter; ++iter) {
				auto pathName = iter->path().generic_string();
				pathNames.insert(std::upper_bound(pathNames.begin(), pathNames.end(), pathName), pathName);
			}

			return pathNames;
		}

		auto LoadTransaction(const std::string& filePath) {
			io::RawFile txFile(filePath, io::OpenMode::Read_Only);
			auto transactionSize = io::Read32(txFile);
			auto pTransaction = utils::MakeSharedWithSize<model::Transaction>(transactionSize);
			pTransaction->Size = transactionSize;
			txFile.read({ reinterpret_cast<uint8_t*>(pTransaction.get()) + sizeof(uint32_t), transactionSize - sizeof(uint32_t) });
			if (txFile.size() != txFile.position())
				CATAPULT_THROW_RUNTIME_ERROR_1("transaction file has invalid size", filePath);

			return pTransaction;
		}

		bool IsSupportedTransactionType(model::EntityType entityType) {
			return model::Entity_Type_Voting_Key_Link == entityType || model::Entity_Type_Vrf_Key_Link == entityType;
		}

		void ValidateTransaction(const model::Transaction& transaction, const std::string& filePath) {
			if (Timestamp(1) != transaction.Deadline)
				CATAPULT_THROW_RUNTIME_ERROR_1("nemesis transactions need to have deadline set to 1", filePath);

			if (Amount(0) != transaction.MaxFee)
				CATAPULT_THROW_RUNTIME_ERROR_1("nemesis transactions need to have max fee set to 0", filePath);

			if (!IsSupportedTransactionType(transaction.Type))
				CATAPULT_THROW_RUNTIME_ERROR_1("currently only link transactions are supported", filePath);
		}
	}

	std::vector<std::shared_ptr<const model::Transaction>> LoadAndValidateAdditionalTransactions(
			const std::string& transactionsDirectory) {
		std::vector<std::shared_ptr<const model::Transaction>> transactions;
		auto pathNames = GetSortedPathNames(transactionsDirectory);
		for (const auto& filePath : pathNames) {
			auto pTransaction = LoadTransaction(filePath);
			ValidateTransaction(*pTransaction, filePath);
			transactions.push_back(std::move(pTransaction));
		}

		CATAPULT_LOG(info) << "loaded " << transactions.size() << " additional transactions";
		return transactions;
	}
}}}

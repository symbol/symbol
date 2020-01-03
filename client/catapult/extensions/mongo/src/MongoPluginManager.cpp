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

#include "MongoPluginManager.h"

namespace catapult { namespace mongo {

	MongoPluginManager::MongoPluginManager(MongoStorageContext& mongoContext, model::NetworkIdentifier networkIdentifier)
			: m_mongoContext(mongoContext)
			, m_networkIdentifier(networkIdentifier)
	{}

	MongoStorageContext& MongoPluginManager::mongoContext() const {
		return m_mongoContext;
	}

	model::NetworkIdentifier MongoPluginManager::networkIdentifier() const {
		return m_networkIdentifier;
	}

	void MongoPluginManager::addTransactionSupport(std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin) {
		m_transactionRegistry.registerPlugin(std::move(pTransactionPlugin));
	}

	void MongoPluginManager::addReceiptSupport(std::unique_ptr<MongoReceiptPlugin>&& pReceiptPlugin) {
		m_receiptRegistry.registerPlugin(std::move(pReceiptPlugin));
	}

	const MongoTransactionRegistry& MongoPluginManager::transactionRegistry() const {
		return m_transactionRegistry;
	}

	const MongoReceiptRegistry& MongoPluginManager::receiptRegistry() const {
		return m_receiptRegistry;
	}

	std::unique_ptr<ExternalCacheStorage> MongoPluginManager::createStorage() {
		return m_storageBuilder.build();
	}
}}

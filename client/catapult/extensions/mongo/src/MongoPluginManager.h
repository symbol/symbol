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

#pragma once
#include "ExternalCacheStorageBuilder.h"
#include "MongoReceiptPlugin.h"
#include "MongoStorageContext.h"
#include "MongoTransactionPlugin.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/plugins.h"

namespace catapult { namespace mongo {

	/// Manager for registering mongo plugins.
	class PLUGIN_API_DEPENDENCY MongoPluginManager {
	public:
		/// Creates a new plugin manager around \a mongoContext and \a networkIdentifier.
		MongoPluginManager(MongoStorageContext& mongoContext, model::NetworkIdentifier networkIdentifier);

	public:
		/// Gets the mongo storage context.
		MongoStorageContext& mongoContext() const;

		/// Gets the network idenfifier.
		model::NetworkIdentifier networkIdentifier() const;

	public:
		/// Adds support for a transaction described by \a pTransactionPlugin.
		void addTransactionSupport(std::unique_ptr<MongoTransactionPlugin>&& pTransactionPlugin);

		/// Adds support for a receipt described by \a pReceiptPlugin.
		void addReceiptSupport(std::unique_ptr<MongoReceiptPlugin>&& pReceiptPlugin);

		/// Adds support for an external cache storage described by \a pStorage.
		template<typename TStorage>
		void addStorageSupport(std::unique_ptr<TStorage>&& pStorage) {
			m_storageBuilder.add(std::move(pStorage));
		}

	public:
		/// Gets the transaction registry.
		const MongoTransactionRegistry& transactionRegistry() const;

		/// Gets the receipt registry.
		const MongoReceiptRegistry& receiptRegistry() const;

		/// Creates an external cache storage.
		std::unique_ptr<ExternalCacheStorage> createStorage();

	private:
		MongoStorageContext& m_mongoContext;
		model::NetworkIdentifier m_networkIdentifier;
		MongoTransactionRegistry m_transactionRegistry;
		MongoReceiptRegistry m_receiptRegistry;
		ExternalCacheStorageBuilder m_storageBuilder;
	};
}}

/// Entry point for registering a dynamic module with \a manager.
extern "C" PLUGIN_API
void RegisterMongoSubsystem(catapult::mongo::MongoPluginManager& manager);

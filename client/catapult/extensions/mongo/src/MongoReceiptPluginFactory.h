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
#include "MongoReceiptPlugin.h"
#include "mappers/ReceiptMapper.h"
#include "catapult/model/Receipt.h"
#include "catapult/functions.h"

namespace catapult { namespace mongo {

	/// Factory for creating mongo receipt plugins.
	class MongoReceiptPluginFactory {
	private:
		using ReceiptPlugin = MongoReceiptPlugin;

	public:
		/// Creates a receipt plugin around \a type and \a streamFunc.
		template<typename TReceipt, typename TStreamFunc>
		static std::unique_ptr<ReceiptPlugin> Create(model::ReceiptType type, TStreamFunc streamFunc) {
			return std::make_unique<ReceiptPluginT<TReceipt>>(type, streamFunc);
		}

	private:
		template<typename TReceipt>
		class ReceiptPluginT : public ReceiptPlugin {
		private:
			using StreamFunc = consumer<bsoncxx::builder::stream::document&, const TReceipt&>;

		public:
			ReceiptPluginT(model::ReceiptType type, const StreamFunc& streamFunc)
					: m_type(type)
					, m_streamFunc(streamFunc)
			{}

		public:
			model::ReceiptType type() const override {
				return m_type;
			}

			void streamReceipt(bsoncxx::builder::stream::document& builder, const model::Receipt& receipt) const override {
				m_streamFunc(builder, static_cast<const TReceipt&>(receipt));
			}

		private:
			model::ReceiptType m_type;
			StreamFunc m_streamFunc;
		};
	};

/// Defines a mongo receipt plugin factory for \a NAME receipt using \a STREAM.
#define DEFINE_MONGO_RECEIPT_PLUGIN_FACTORY(NAME, STREAM) \
	std::unique_ptr<MongoReceiptPlugin> Create##NAME##ReceiptMongoPlugin(model::ReceiptType type) { \
		return MongoReceiptPluginFactory::Create<model::NAME##Receipt>(type, STREAM); \
	}

	/// Creates a mongo balance transfer receipt plugin around \a type.
	std::unique_ptr<MongoReceiptPlugin> CreateBalanceTransferReceiptMongoPlugin(model::ReceiptType type);

	/// Creates a mongo balance change receipt plugin around \a type.
	std::unique_ptr<MongoReceiptPlugin> CreateBalanceChangeReceiptMongoPlugin(model::ReceiptType type);

	/// Creates a mongo inflation receipt plugin around \a type.
	std::unique_ptr<MongoReceiptPlugin> CreateInflationReceiptMongoPlugin(model::ReceiptType type);

	/// Creates a mongo artifact expiry receipt plugin around \a type.
	template<typename TArtifactId>
	std::unique_ptr<MongoReceiptPlugin> CreateArtifactExpiryReceiptMongoPlugin(model::ReceiptType type) {
		return MongoReceiptPluginFactory::Create<model::ArtifactExpiryReceipt<TArtifactId>>(type, mappers::StreamReceipt<TArtifactId>);
	}
}}

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
#include "catapult/model/PluginRegistry.h"
#include "catapult/model/ReceiptType.h"
#include "catapult/plugins.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>

namespace catapult { namespace model {
	struct Receipt;
}}

namespace catapult { namespace mongo {

	/// Mongo receipt plugin.
	class MongoReceiptPlugin {
	public:
		virtual ~MongoReceiptPlugin() = default;

	public:
		/// Gets the receipt type.
		virtual model::ReceiptType type() const = 0;

		/// Streams \a receipt to \a builder.
		virtual void streamReceipt(bsoncxx::builder::stream::document& builder, const model::Receipt& receipt) const = 0;
	};

	/// Registry of mongo receipt plugins.
	class MongoReceiptRegistry : public model::PluginRegistry<MongoReceiptPlugin, model::ReceiptType> {};
}}

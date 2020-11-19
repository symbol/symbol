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

#include "MockReceiptMapper.h"
#include "mongo/src/mappers/MapperUtils.h"

namespace catapult { namespace mocks {

	using namespace catapult::mongo::mappers;
	using MongoReceiptPlugin = mongo::MongoReceiptPlugin;

	namespace {
		class MockMongoReceiptPlugin : public MongoReceiptPlugin {
		public:
			explicit MockMongoReceiptPlugin(model::ReceiptType type) : m_type(type)
			{}

		public:
			model::ReceiptType type() const override {
				return m_type;
			}

			void streamReceipt(bson_stream::document& builder, const model::Receipt& receipt) const override {
				const auto& mockReceipt = static_cast<const MockReceipt&>(receipt);
				builder << "mock_payload" << ToBinary(mockReceipt.Payload.data(), mockReceipt.Payload.size());
			}

		private:
			model::ReceiptType m_type;
		};
	}

	std::unique_ptr<MongoReceiptPlugin> CreateMockReceiptMongoPlugin(int type) {
		return std::make_unique<MockMongoReceiptPlugin>(static_cast<model::ReceiptType>(type));
	}
}}

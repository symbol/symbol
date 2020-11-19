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

#include "AddressExtractionUtChangeSubscriber.h"
#include "AddressExtractor.h"

namespace catapult { namespace addressextraction {

	namespace {
		class AddressExtractionUtChangeSubscriber : public cache::UtChangeSubscriber {
		public:
			explicit AddressExtractionUtChangeSubscriber(const AddressExtractor& extractor) : m_extractor(extractor)
			{}

		public:
			void notifyAdds(const TransactionInfos& transactionInfos) override {
				m_extractor.extract(const_cast<TransactionInfos&>(transactionInfos));
			}

			void notifyRemoves(const TransactionInfos& transactionInfos) override {
				m_extractor.extract(const_cast<TransactionInfos&>(transactionInfos));
			}

			void flush() override {
				// nothing to intercept
			}

		private:
			const AddressExtractor& m_extractor;
		};
	}

	std::unique_ptr<cache::UtChangeSubscriber> CreateAddressExtractionUtChangeSubscriber(const AddressExtractor& extractor) {
		return std::make_unique<AddressExtractionUtChangeSubscriber>(extractor);
	}
}}

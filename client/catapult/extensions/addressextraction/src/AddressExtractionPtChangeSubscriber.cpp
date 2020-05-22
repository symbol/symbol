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

#include "AddressExtractionPtChangeSubscriber.h"
#include "AddressExtractor.h"

namespace catapult { namespace addressextraction {

	namespace {
		class AddressExtractionPtChangeSubscriber : public cache::PtChangeSubscriber {
		public:
			explicit AddressExtractionPtChangeSubscriber(const AddressExtractor& extractor) : m_extractor(extractor)
			{}

		public:
			void notifyAddPartials(const TransactionInfos& transactionInfos) override {
				m_extractor.extract(const_cast<TransactionInfos&>(transactionInfos));
			}

			void notifyAddCosignature(const model::TransactionInfo& parentTransactionInfo, const model::Cosignature&) override {
				m_extractor.extract(const_cast<model::TransactionInfo&>(parentTransactionInfo));
			}

			void notifyRemovePartials(const TransactionInfos& transactionInfos) override {
				m_extractor.extract(const_cast<TransactionInfos&>(transactionInfos));
			}

			void flush() override {
				// nothing to intercept
			}

		private:
			const AddressExtractor& m_extractor;
		};
	}

	std::unique_ptr<cache::PtChangeSubscriber> CreateAddressExtractionPtChangeSubscriber(const AddressExtractor& extractor) {
		return std::make_unique<AddressExtractionPtChangeSubscriber>(extractor);
	}
}}

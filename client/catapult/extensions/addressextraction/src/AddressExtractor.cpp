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

#include "AddressExtractor.h"
#include "catapult/model/Elements.h"
#include "catapult/model/TransactionUtils.h"

namespace catapult { namespace addressextraction {

	AddressExtractor::AddressExtractor(std::unique_ptr<const model::NotificationPublisher>&& pPublisher)
			: m_pPublisher(std::move(pPublisher))
	{}

	void AddressExtractor::extract(model::TransactionInfo& transactionInfo) const {
		if (transactionInfo.OptionalExtractedAddresses)
			return;

		auto addresses = model::ExtractAddresses(*transactionInfo.pEntity, *m_pPublisher);
		transactionInfo.OptionalExtractedAddresses = std::make_shared<model::UnresolvedAddressSet>(std::move(addresses));
	}

	void AddressExtractor::extract(model::TransactionInfosSet& transactionInfos) const {
		for (auto& transactionInfo : transactionInfos)
			extract(const_cast<model::TransactionInfo&>(transactionInfo));
	}

	void AddressExtractor::extract(model::TransactionElement& transactionElement) const {
		if (transactionElement.OptionalExtractedAddresses)
			return;

		auto addresses = model::ExtractAddresses(transactionElement.Transaction, *m_pPublisher);
		transactionElement.OptionalExtractedAddresses = std::make_shared<model::UnresolvedAddressSet>(std::move(addresses));
	}

	void AddressExtractor::extract(model::BlockElement& blockElement) const {
		for (auto& transactionElement : blockElement.Transactions)
			extract(transactionElement);
	}
}}

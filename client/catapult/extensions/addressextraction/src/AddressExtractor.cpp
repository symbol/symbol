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

	namespace {
		void AddResolvedAddresses(
				model::AddressSet& addresses,
				uint32_t primaryId,
				const model::AddressResolutionStatement& resolutionStatement) {
			for (auto i = 0u; i < resolutionStatement.size(); ++i) {
				const auto& resolutionEntry = resolutionStatement.entryAt(i);

				// resolution statements are ordered
				if (primaryId < resolutionEntry.Source.PrimaryId)
					break;

				if (primaryId > resolutionEntry.Source.PrimaryId)
					continue;

				addresses.insert(resolutionEntry.ResolvedValue);
			}
		}

		model::AddressSet FindResolvedAddresses(
				const decltype(model::BlockStatement::AddressResolutionStatements)& addressResolutionStatements,
				uint32_t primaryId,
				const model::UnresolvedAddressSet& extractedAddresses) {
			model::AddressSet resolvedAddresses;
			for (const auto& address : extractedAddresses) {
				auto resolutionStatementIter = addressResolutionStatements.find(address);
				if (addressResolutionStatements.cend() != resolutionStatementIter)
					AddResolvedAddresses(resolvedAddresses, primaryId, resolutionStatementIter->second);
			}

			return resolvedAddresses;
		}

		void UpdateExtractedAddresses(model::TransactionElement& transactionElement, const model::AddressSet& resolvedAddresses) {
			if (resolvedAddresses.empty())
				return;

			auto pMergedAddresses = std::make_shared<model::UnresolvedAddressSet>();
			*pMergedAddresses = *transactionElement.OptionalExtractedAddresses;

			for (const auto& address : resolvedAddresses)
				pMergedAddresses->insert(address.copyTo<UnresolvedAddress>());

			transactionElement.OptionalExtractedAddresses = pMergedAddresses;
		}
	}

	void AddressExtractor::extract(model::BlockElement& blockElement) const {
		auto primaryId = 1u; // transaction primary identifiers are 1-based

		for (auto& transactionElement : blockElement.Transactions) {
			extract(transactionElement);

			if (blockElement.OptionalStatement) {
				auto resolvedAddresses = FindResolvedAddresses(
						blockElement.OptionalStatement->AddressResolutionStatements,
						primaryId,
						*transactionElement.OptionalExtractedAddresses);
				UpdateExtractedAddresses(transactionElement, resolvedAddresses);
			}

			++primaryId;
		}
	}
}}

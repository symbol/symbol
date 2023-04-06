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
			auto shouldAddPreviousResolution = true;
			for (auto i = 0u; i < resolutionStatement.size(); ++i) {
				const auto& resolutionEntry = resolutionStatement.entryAt(resolutionStatement.size() - i - 1);

				// skip future resolutions (not yet effective)
				if (primaryId < resolutionEntry.Source.PrimaryId)
					continue;

				// add all new resolutions triggered by the current transaction
				if (primaryId == resolutionEntry.Source.PrimaryId) {
					addresses.insert(resolutionEntry.ResolvedValue);

					// in the edge case of an aggregate (nonzero SecondaryId) containing steps 2-4 below,
					// there is not enough information to determine whether or not 1 has occured:
					//     1. old alias is used
					//     2. alias is unlinked
					//     3. new alias is linked
					//     4. new alias is used
					// in this ambiguous situation, include the old alias (potential false positive)

					// for a non-aggregate transaction, a single unresolved address can never map multiple resolved addresses
					if (!resolutionEntry.Source.SecondaryId)
						shouldAddPreviousResolution = false;
				}

				// if there is an active resolution from a previous transaction, add it
				if (primaryId > resolutionEntry.Source.PrimaryId) {
					if (shouldAddPreviousResolution)
						addresses.insert(resolutionEntry.ResolvedValue);

					break;
				}
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

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
#include "Cosignature.h"
#include "Transaction.h"
#include <vector>

namespace catapult { namespace model {

	/// Wrapper around a transaction and its cosignatures.
	class WeakCosignedTransactionInfo {
	public:
		/// Creates an empty weak transaction info.
		WeakCosignedTransactionInfo() : WeakCosignedTransactionInfo(nullptr, nullptr)
		{}

		/// Creates a weak transaction info around \a pTransaction and \a pCosignatures.
		WeakCosignedTransactionInfo(const Transaction* pTransaction, const std::vector<Cosignature>* pCosignatures)
				: m_pTransaction(pTransaction)
				, m_pCosignatures(pCosignatures)
		{}

	public:
		/// Gets the transaction.
		const Transaction& transaction() const {
			return *m_pTransaction;
		}

		/// Gets the cosignatures.
		const std::vector<Cosignature>& cosignatures() const {
			return *m_pCosignatures;
		}

		/// Returns \c true if a cosignature from \a signer is present.
		bool hasCosignatory(const Key& signer) const {
			return std::any_of(m_pCosignatures->cbegin(), m_pCosignatures->cend(), [&signer](const auto& cosignature) {
				return signer == cosignature.SignerPublicKey;
			});
		}

	public:
		/// Returns \c true if the info is not empty and contains a valid entity pointer, \c false otherwise.
		explicit operator bool() const noexcept {
			return !!m_pTransaction;
		}

	private:
		const Transaction* m_pTransaction;
		const std::vector<Cosignature>* m_pCosignatures;
	};
}}

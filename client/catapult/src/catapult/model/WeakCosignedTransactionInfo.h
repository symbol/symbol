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
		bool hasCosigner(const Key& signer) const {
			return std::any_of(m_pCosignatures->cbegin(), m_pCosignatures->cend(), [&signer](const auto& cosignature) {
				return signer == cosignature.Signer;
			});
		}

	public:
		/// Returns \c true if the info is non-empty and contains a valid entity pointer, \c false otherwise.
		explicit operator bool() const noexcept {
			return !!m_pTransaction;
		}

	private:
		const Transaction* m_pTransaction;
		const std::vector<Cosignature>* m_pCosignatures;
	};
}}

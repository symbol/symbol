#pragma once
#include "catapult/model/NetworkInfo.h"
#include "catapult/model/Transaction.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace builders {

	/// Base transaction builder.
	template<typename TTransaction>
	class TransactionBuilder {
	public:
		/// Creates a transaction builder with \a networkIdentifier and \a signer.
		TransactionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
				: m_networkIdentifier(networkIdentifier)
				, m_signer(signer)
				, m_type(TTransaction::Entity_Type)
				, m_version(MakeVersion(m_networkIdentifier, TTransaction::Current_Version))
		{}

	public:
		/// Sets transaction \a deadline.
		void setDeadline(catapult::Timestamp deadline) {
			m_deadline = deadline;
		}

		/// Sets transaction \a fee.
		void setFee(catapult::Amount fee) {
			m_fee = fee;
		}

	protected:
		std::unique_ptr<TTransaction> createTransaction(size_t size) const {
			std::unique_ptr<TTransaction> pTransaction(reinterpret_cast<TTransaction*>(::operator new(size)));
			std::memset(pTransaction.get(), 0, sizeof(TTransaction));

			// verifiable entity data
			pTransaction->Size = utils::checked_cast<size_t, uint32_t>(size);
			pTransaction->Type = m_type;
			pTransaction->Version = m_version;
			pTransaction->Signer = m_signer;

			// transaction data
			pTransaction->Deadline = m_deadline;
			pTransaction->Fee = m_fee;
			return pTransaction;
		}

	private:
		const model::NetworkIdentifier m_networkIdentifier;
		const Key& m_signer;
		const model::EntityType m_type;
		const uint16_t m_version;

		Timestamp m_deadline;
		Amount m_fee;
	};
}}

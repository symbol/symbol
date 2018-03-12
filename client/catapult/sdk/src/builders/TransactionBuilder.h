#pragma once
#include "catapult/model/NetworkInfo.h"
#include "catapult/model/Transaction.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace builders {

	/// Base transaction builder.
	class TransactionBuilder {
	public:
		/// Creates a transaction builder with \a networkIdentifier and \a signer.
		TransactionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
				: m_networkIdentifier(networkIdentifier)
				, m_signer(signer)
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

	private:
		void setAdditionalFields(model::EmbeddedTransaction&) const {
		}

		void setAdditionalFields(model::Transaction& transaction) const {
			transaction.Deadline = m_deadline;
			transaction.Fee = m_fee;
		}

	protected:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> createTransaction(size_t size) const {
			auto pTransaction = utils::MakeUniqueWithSize<TTransaction>(size);
			std::memset(pTransaction.get(), 0, sizeof(TTransaction));

			// verifiable entity data
			pTransaction->Size = utils::checked_cast<size_t, uint32_t>(size);
			pTransaction->Type = TTransaction::Entity_Type;
			pTransaction->Version = MakeVersion(m_networkIdentifier, TTransaction::Current_Version);
			pTransaction->Signer = m_signer;

			// transaction data
			setAdditionalFields(*pTransaction);
			return pTransaction;
		}

	private:
		const model::NetworkIdentifier m_networkIdentifier;
		const Key& m_signer;

		Timestamp m_deadline;
		Amount m_fee;
	};
}}

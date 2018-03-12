#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/multisig/src/model/ModifyMultisigAccountTransaction.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a modify multisig account transaction.
	class ModifyMultisigAccountBuilder : public TransactionBuilder {
	public:
		using Transaction = model::ModifyMultisigAccountTransaction;
		using EmbeddedTransaction = model::EmbeddedModifyMultisigAccountTransaction;

		/// Creates a modify multisig account builder for building a modify multisig account transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		ModifyMultisigAccountBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the relative change of the minimal number of cosignatories (\a minRemovalDelta)
		/// required when removing an account.
		void setMinRemovalDelta(int8_t minRemovalDelta);

		/// Sets the relative change of the minimal number of cosignatories (\a minApprovalDelta)
		/// required when approving a transaction.
		void setMinApprovalDelta(int8_t minApprovalDelta);

		/// Adds a cosignatory modification around \a type and \a key.
		void addCosignatoryModification(model::CosignatoryModificationType type, const Key& key);

	public:
		/// Builds a new modify multisig account transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded modify multisig account transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		int8_t m_minRemovalDelta;
		int8_t m_minApprovalDelta;
		std::vector<model::CosignatoryModification> m_modifications;
	};
}}

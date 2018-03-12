#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/namespace/src/model/RegisterNamespaceTransaction.h"
#include "catapult/model/NetworkInfo.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a register namespace transaction.
	class RegisterNamespaceBuilder : public TransactionBuilder {
	public:
		using Transaction = model::RegisterNamespaceTransaction;
		using EmbeddedTransaction = model::EmbeddedRegisterNamespaceTransaction;

		/// Creates a register namespace builder for building a namespace registration transaction for \a name
		/// from \a signer for the network specified by \a networkIdentifier.
		RegisterNamespaceBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer, const RawString& name);

	public:
		/// Sets the namespace \a duration.
		void setDuration(BlockDuration duration);

		/// Sets the namespace parent id (\a parentId).
		void setParentId(NamespaceId parentId);

	public:
		/// Builds a new register namespace transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded register namespace transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		// properties
		NamespaceId m_parentId;
		std::string m_name;
		BlockDuration m_duration;
	};
}}

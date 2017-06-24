#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/namespace/src/model/RegisterNamespaceTransaction.h"
#include "catapult/model/NetworkInfo.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a register namespace transaction.
	class RegisterNamespaceBuilder : public TransactionBuilder<model::RegisterNamespaceTransaction> {
	public:
		/// Creates a register namespace builder for building a namespace registration transaction for \a name
		/// from \a signer for the network specified by \a networkIdentifier.
		RegisterNamespaceBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer, const RawString& name);

	public:
		/// Sets the namespace \a duration.
		void setDuration(ArtifactDuration duration);

		/// Sets the namespace parent id (\a parentId).
		void setParentId(NamespaceId parentId);

	public:
		/// Builds a new register namespace transaction.
		std::unique_ptr<model::RegisterNamespaceTransaction> build() const;

	private:
		// properties
		NamespaceId m_parentId;
		std::string m_name;
		ArtifactDuration m_duration;
	};
}}

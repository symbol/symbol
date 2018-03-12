#include "RegisterNamespaceBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "plugins/txes/namespace/src/model/NamespaceConstants.h"

namespace catapult { namespace builders {

	RegisterNamespaceBuilder::RegisterNamespaceBuilder(
			model::NetworkIdentifier networkIdentifier,
			const Key& signer,
			const RawString& name)
			: TransactionBuilder(networkIdentifier, signer)
			, m_name(name.pData, name.Size) {
		if (m_name.empty())
			CATAPULT_THROW_INVALID_ARGUMENT("cannot set empty name");
	}

	void RegisterNamespaceBuilder::setDuration(BlockDuration duration) {
		m_duration = duration;
	}

	void RegisterNamespaceBuilder::setParentId(NamespaceId parentId) {
		m_parentId = parentId;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> RegisterNamespaceBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType) + m_name.size();
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set transaction fields
		if (Namespace_Base_Id == m_parentId) {
			pTransaction->NamespaceType = model::NamespaceType::Root;
			pTransaction->Duration = m_duration;
		} else {
			pTransaction->NamespaceType = model::NamespaceType::Child;
			pTransaction->ParentId = m_parentId;
		}

		pTransaction->NamespaceId = model::GenerateNamespaceId(m_parentId, m_name);

		// 3. set name
		pTransaction->NamespaceNameSize = utils::checked_cast<size_t, uint8_t>(m_name.size());
		std::copy(m_name.cbegin(), m_name.cend(), pTransaction->NamePtr());
		return pTransaction;
	}

	std::unique_ptr<RegisterNamespaceBuilder::Transaction> RegisterNamespaceBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<RegisterNamespaceBuilder::EmbeddedTransaction> RegisterNamespaceBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}

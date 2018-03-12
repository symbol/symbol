#include "MosaicDefinitionBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"

namespace catapult { namespace builders {

	MosaicDefinitionBuilder::MosaicDefinitionBuilder(
			model::NetworkIdentifier networkIdentifier,
			const Key& signer,
			NamespaceId parentId,
			const RawString& name)
			: TransactionBuilder(networkIdentifier, signer)
			, m_parentId(parentId)
			, m_name(name.pData, name.pData + name.Size)
			, m_flags(model::MosaicFlags::None)
			, m_divisibility(0) {
		if (m_name.empty())
			CATAPULT_THROW_INVALID_ARGUMENT("cannot set empty name");
	}

	void MosaicDefinitionBuilder::setSupplyMutable() {
		m_flags |= model::MosaicFlags::Supply_Mutable;
	}

	void MosaicDefinitionBuilder::setTransferable() {
		m_flags |= model::MosaicFlags::Transferable;
	}

	void MosaicDefinitionBuilder::setLevyMutable() {
		m_flags |= model::MosaicFlags::Levy_Mutable;
	}

	void MosaicDefinitionBuilder::setDivisibility(uint8_t divisibility) {
		m_divisibility = divisibility;
	}

	void MosaicDefinitionBuilder::setDuration(BlockDuration duration) {
		// drop if 'default duration'
		if (Eternal_Artifact_Duration == duration)
			dropOptionalProperty(model::MosaicPropertyId::Duration);
		else
			addOptionalProperty(model::MosaicPropertyId::Duration, duration.unwrap());
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> MosaicDefinitionBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto propertiesSize = sizeof(model::MosaicProperty) * m_optionalProperties.size();
		auto size = sizeof(TransactionType) + propertiesSize + m_name.size();
		auto pTransaction = createTransaction<TransactionType>(size);

		// 2. set transaction fields
		pTransaction->ParentId = m_parentId;
		pTransaction->MosaicId = model::GenerateMosaicId(m_parentId, m_name);

		pTransaction->PropertiesHeader.Flags = m_flags;
		pTransaction->PropertiesHeader.Divisibility = m_divisibility;

		// 3. set sizes upfront, so that pointers are calculated correctly
		pTransaction->PropertiesHeader.Count = utils::checked_cast<size_t, uint8_t>(m_optionalProperties.size());
		pTransaction->MosaicNameSize = utils::checked_cast<size_t, uint8_t>(m_name.size());

		// 4. set optional properties
		auto* pProperties = pTransaction->PropertiesPtr();
		for (const auto& propertyPair : m_optionalProperties) {
			*pProperties = model::MosaicProperty{ propertyPair.first, propertyPair.second };
			++pProperties;
		}

		// 5. set name
		std::copy(m_name.cbegin(), m_name.cend(), pTransaction->NamePtr());
		return pTransaction;
	}

	std::unique_ptr<MosaicDefinitionBuilder::Transaction> MosaicDefinitionBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<MosaicDefinitionBuilder::EmbeddedTransaction> MosaicDefinitionBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}

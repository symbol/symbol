#include "TransferBuilder.h"
#include "src/extensions/IdGenerator.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace builders {

	namespace {
		RawBuffer RawStringToRawBuffer(const RawString& str) {
			return RawBuffer(reinterpret_cast<const uint8_t*>(str.pData), str.Size);
		}
	}

	TransferBuilder::TransferBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer, const Address& recipient)
			: TransactionBuilder(networkIdentifier, signer)
			, m_recipient(recipient)
	{}

	void TransferBuilder::setMessage(const RawBuffer& message) {
		if (0 == message.Size)
			CATAPULT_THROW_INVALID_ARGUMENT("cannot set empty message");

		if (!m_message.empty())
			CATAPULT_THROW_RUNTIME_ERROR("message was already set");

		m_message.resize(message.Size);
		m_message.assign(message.pData, message.pData + message.Size);
	}

	void TransferBuilder::setStringMessage(const RawString& message) {
		setMessage(RawStringToRawBuffer(message));
	}

	void TransferBuilder::addMosaic(MosaicId mosaicId, Amount amount) {
		if (m_mosaicTransfers.cend() != m_mosaicTransfers.find(mosaicId))
			CATAPULT_THROW_RUNTIME_ERROR_1("mosaic was already added", mosaicId);

		m_mosaicTransfers.emplace(mosaicId, amount);
	}

	void TransferBuilder::addMosaic(const RawString& mosaicName, Amount amount) {
		auto mosaicId = extensions::GenerateMosaicId(mosaicName);
		addMosaic(mosaicId, amount);
	}

	std::unique_ptr<model::TransferTransaction> TransferBuilder::build() const {
		using TransactionType = model::TransferTransaction;

		// 1. allocate, zero (header), set model::Transaction fields
		auto size = sizeof(TransactionType) + m_message.size();
		size += m_mosaicTransfers.size() * sizeof(model::Mosaic);
		auto pTransaction = createTransaction(size);

		// 2. set transaction fields
		pTransaction->Recipient = m_recipient;

		// 3. set sizes upfront, so that pointers are calculated correctly
		pTransaction->MessageSize = utils::checked_cast<size_t, uint16_t>(m_message.size());
		pTransaction->MosaicsCount = utils::checked_cast<size_t, uint8_t>(m_mosaicTransfers.size());

		// 4. set message
		if (!m_message.empty())
			std::copy(m_message.cbegin(), m_message.cend(), pTransaction->MessagePtr());

		// 5. set mosaics
		if (!m_mosaicTransfers.empty()) {
			auto* pMosaic = pTransaction->MosaicsPtr();
			for (const auto& mosaicTransfer : m_mosaicTransfers) {
				pMosaic->MosaicId = mosaicTransfer.first;
				pMosaic->Amount = mosaicTransfer.second;
				++pMosaic;
			}
		}

		return pTransaction;
	}
}}

#pragma once
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/utils/Hashers.h"
#include <unordered_map>

namespace catapult { namespace extensions {

	/// A notification subscriber for observering nemesis balance transfers.
	class NemesisBalanceTransferSubscriber : public model::NotificationSubscriber {
	public:
		/// Map of mosaic ids to amounts.
		using BalanceTransfers = std::unordered_map<MosaicId, Amount, utils::BaseValueHasher<MosaicId>>;

	public:
		/// Creates a subscriber for nemesis account with \a nemesisPublicKey.
		explicit NemesisBalanceTransferSubscriber(const Key& nemesisPublicKey) : m_nemesisPublicKey(nemesisPublicKey)
		{}

	public:
		void notify(const model::Notification& notification) override;

	private:
		void notify(const model::BalanceTransferNotification& notification);
		void notify(const Key& sender, const Address& recipient, MosaicId mosaicId, Amount amount);

	public:
		/// Gets collected nemesis outflows.
		const BalanceTransfers& outflows() const {
			return m_outflows;
		}

	private:
		Key m_nemesisPublicKey;
		BalanceTransfers m_outflows;
	};
}}

#include "NemesisBalanceTransferSubscriber.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"

namespace catapult { namespace extensions {

	void NemesisBalanceTransferSubscriber::notify(const model::Notification& notification) {
		if (model::BalanceTransferNotification::Notification_Type == notification.Type)
			notify(static_cast<const model::BalanceTransferNotification&>(notification));
	}

	void NemesisBalanceTransferSubscriber::notify(const model::BalanceTransferNotification& notification) {
		notify(notification.Sender, notification.Recipient, notification.MosaicId, notification.Amount);
	}

	void NemesisBalanceTransferSubscriber::notify(const Key& sender, const Address&, MosaicId mosaicId, Amount amount) {
		if (m_nemesisPublicKey != sender)
			CATAPULT_THROW_INVALID_ARGUMENT_1("all nemesis outflows must originate from nemesis account", utils::HexFormat(sender));

		auto iter = m_outflows.find(mosaicId);
		if (m_outflows.end() == iter)
			m_outflows.emplace(mosaicId, amount);
		else
			iter->second = iter->second + amount;
	}
}}

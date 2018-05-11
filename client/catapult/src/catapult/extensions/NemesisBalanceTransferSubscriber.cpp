/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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

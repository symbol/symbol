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

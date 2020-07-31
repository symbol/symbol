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

#include "Observers.h"

namespace catapult { namespace observers {

	DEFINE_OBSERVER(SourceChange, model::SourceChangeNotification, [](
			const model::SourceChangeNotification& notification,
			ObserverContext& context) {
		// source is only changed during commit (never rollback) because it is only used for supplemental receipt generation;
		// never any intrinsic state changes
		if (NotifyMode::Commit != context.Mode)
			return;

		auto& statementBuilder = context.StatementBuilder();
		model::ReceiptSource newSource(notification.PrimaryId, notification.SecondaryId);
		const auto& currentSource = statementBuilder.source();
		if (model::SourceChangeNotification::SourceChangeType::Relative == notification.PrimaryChangeType)
			newSource.PrimaryId += currentSource.PrimaryId;

		if (model::SourceChangeNotification::SourceChangeType::Relative == notification.SecondaryChangeType)
			newSource.SecondaryId += currentSource.SecondaryId;

		statementBuilder.setSource(newSource);
	})
}}

/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "FinalizationTestUtils.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	FinalizationNotification GenerateRandomFinalizationNotification() {
		FinalizationNotification notification;
		notification.Round = { FinalizationEpoch(static_cast<uint32_t>(Random())), FinalizationPoint(static_cast<uint32_t>(Random())) };
		notification.Height = Height(Random());
		FillWithRandomData(notification.Hash);
		return notification;
	}

	void WriteFinalizationNotification(io::OutputStream& outputStream, const FinalizationNotification& notification) {
		outputStream.write(notification.Hash);
		outputStream.write({ reinterpret_cast<const uint8_t*>(&notification.Round), sizeof(model::FinalizationRound) });
		io::Write(outputStream, notification.Height);
	}
}}

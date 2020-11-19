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

#include "WaitFunctions.h"
#include "tests/test/nodeps/Waits.h"
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>

namespace catapult { namespace test {

	WaitFunction CreateSyncWaitFunction(uint32_t waitMillis) {
		return [waitMillis](const auto&, const auto& shouldWait) {
			while (shouldWait())
				Sleep(waitMillis);
		};
	}

	namespace {
		void WaitAsync(boost::asio::io_context& ioContext, const ShouldWaitPredicate& shouldWait, uint32_t waitMillis) {
			if (!shouldWait())
				return;

			auto pTimer = std::make_shared<boost::asio::steady_timer>(ioContext);
			pTimer->expires_from_now(std::chrono::milliseconds(waitMillis));
			pTimer->async_wait([&ioContext, shouldWait, waitMillis](const auto&) {
				WaitAsync(ioContext, shouldWait, waitMillis);
			});
		}
	}

	WaitFunction CreateAsyncWaitFunction(uint32_t waitMillis) {
		return [waitMillis](auto& ioContext, const auto& shouldWait) {
			WaitAsync(ioContext, shouldWait, waitMillis);
		};
	}
}}

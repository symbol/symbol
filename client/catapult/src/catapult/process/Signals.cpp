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

#include "Signals.h"
#include "catapult/utils/Logging.h"
#include <boost/asio.hpp>

namespace catapult { namespace process {

	namespace {
		struct SignalInfo {
			boost::system::error_code Error;
			int Id;
		};
	}

	void WaitForTerminationSignal() {
		boost::asio::io_context ioContext;
		boost::asio::signal_set signals(ioContext, SIGINT, SIGTERM);

		SignalInfo info;
		signals.async_wait([&info](const auto& ec, auto signalId) {
			info.Error = ec;
			info.Id = signalId;
		});

		CATAPULT_LOG(info) << "waiting for termination signal";
		ioContext.run();

		if (info.Error)
			CATAPULT_LOG(warning) << "error waiting for termination signal: " << info.Error;
		else
			CATAPULT_LOG(info) << "termination signal " << info.Id << " received";
	}
}}

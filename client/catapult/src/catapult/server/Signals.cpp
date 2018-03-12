#include "Signals.h"
#include "catapult/utils/Logging.h"
#include <boost/asio.hpp>

namespace catapult { namespace server {

	namespace {
		struct SignalInfo {
			boost::system::error_code Error;
			int Id;
		};
	}

	void WaitForTerminationSignal() {
		boost::asio::io_service service;
		boost::asio::signal_set signals(service, SIGINT, SIGTERM);

		SignalInfo info;
		signals.async_wait([&info](const auto& ec, auto signalId) {
			info.Error = ec;
			info.Id = signalId;
		});

		CATAPULT_LOG(info) << "waiting for termination signal";
		service.run();

		if (info.Error)
			CATAPULT_LOG(warning) << "error waiting for termination signal: " << info.Error;
		else
			CATAPULT_LOG(info) << "termination signal " << info.Id << " received";
	}
}}

#include "Logging.h"
#include <boost/log/sinks.hpp>
#include <boost/phoenix.hpp>

namespace catapult { namespace test {

	GlobalLogFilter::GlobalLogFilter(utils::LogLevel level) {
		auto filter = boost::phoenix::bind([level](const auto& severity) {
			return severity >= static_cast<boost::log::trivial::severity_level>(level);
		}, boost::log::trivial::severity.or_throw());

		auto pCore = boost::log::core::get();
		pCore->set_filter(filter);
	}

	GlobalLogFilter::~GlobalLogFilter() {
		auto pCore = boost::log::core::get();
		pCore->reset_filter();
	}
}}

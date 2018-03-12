#pragma once
#include <boost/exception/diagnostic_information.hpp>

namespace catapult { namespace utils {

/// Outputs exception diagnostic information on a new line.
#define EXCEPTION_DIAGNOSTIC_MESSAGE() std::endl << boost::current_exception_diagnostic_information()

/// Outputs a message for an unhandled exception that occurred during \a ACTION.
#define UNHANDLED_EXCEPTION_MESSAGE(ACTION) "unhandled exception while " << ACTION << "!" << EXCEPTION_DIAGNOSTIC_MESSAGE()
}}


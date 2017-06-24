#pragma once
#include "catapult/exceptions.h"

namespace catapult { namespace plugins {

	/// Exception class that is thrown when an error is encountered while loading a plugin.
	class plugin_load_error : public catapult_runtime_error {
	public:
		using catapult_runtime_error::catapult_runtime_error;
	};
}}

#pragma once
#include "catapult/exceptions.h"

namespace catapult { namespace api {

	/// Exception class that is thrown when a catapult api operation produces an error.
	class catapult_api_error : public catapult_runtime_error {
	public:
		using catapult_runtime_error::catapult_runtime_error;
	};
}}

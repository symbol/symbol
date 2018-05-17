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
#include "FacilityCode.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/Logging.h"
#include <iosfwd>

namespace catapult { namespace validators {

	/// Possible result severities (only two bits are used).
	enum class ResultSeverity : uint8_t {
		/// Validation result is success.
		Success = 0,
		/// Validation result is neither success nor failure.
		Neutral = 1,
		/// Validation result is failure.
		Failure = 2
	};

	/// Possible result flags (only six bits are used).
	enum class ResultFlags : uint8_t {
		/// No special result flags are set.
		None,
		/// Result is verbose and should be suppressed from most logs.
		Verbose = 1
	};

	/// Enumeration of all possible validation results.
	enum class ValidationResult : uint32_t {
		/// Validation succeeded.
		Success = 0x00000000,
		/// Validation is neutral.
		Neutral = 0x40000000,
		/// Validation failed.
		Failure = 0x80000000
	};

	/// Makes a validation result given \a severity, \a facility, \a code and \a flags.
	constexpr ValidationResult MakeValidationResult(ResultSeverity severity, FacilityCode facility, uint16_t code, ResultFlags flags) {
		return static_cast<ValidationResult>(
				(static_cast<uint32_t>(severity) & 0x03) << 30 | // 01..02: severity
				(static_cast<uint32_t>(flags) & 0x3F) << 24 | //    03..08: flags
				static_cast<uint32_t>(facility) << 16 | //          09..16: facility
				code); //                                           16..32: code
	}

/// Defines a validation result given \a SEVERITY, \a FACILITY, \a DESCRIPTION, \a CODE and \a FLAGS.
#define DEFINE_VALIDATION_RESULT(SEVERITY, FACILITY, DESCRIPTION, CODE, FLAGS) \
	constexpr auto SEVERITY##_##FACILITY##_##DESCRIPTION = validators::MakeValidationResult( \
			(validators::ResultSeverity::SEVERITY), \
			(validators::FacilityCode::FACILITY), \
			CODE, \
			(validators::ResultFlags::FLAGS))

	/// Extracts the encoded result severity from \a result.
	constexpr ResultSeverity GetSeverity(ValidationResult result) {
		return static_cast<ResultSeverity>(utils::to_underlying_type(result) >> 30);
	}

	/// Checks if \a result has all \a flags set.
	constexpr bool IsSet(ValidationResult result, ResultFlags flags) {
		return utils::to_underlying_type(flags) == (utils::to_underlying_type(flags) & (utils::to_underlying_type(result) >> 24 & 0x3F));
	}

	/// Returns a value indicating whether the validation \a result is a success.
	constexpr bool IsValidationResultSuccess(ValidationResult result) {
		return ResultSeverity::Success == GetSeverity(result);
	}

	/// Returns a value indicating whether the validation \a result is a failure.
	constexpr bool IsValidationResultFailure(ValidationResult result) {
		return ResultSeverity::Failure == GetSeverity(result);
	}

	/// Maps validation \a result to an appropriate logging level.
	constexpr utils::LogLevel MapToLogLevel(ValidationResult result) {
		return IsSet(result, ResultFlags::Verbose) ? utils::LogLevel::Trace : utils::LogLevel::Warning;
	}

	/// Insertion operator for outputting \a result to \a out.
	std::ostream& operator<<(std::ostream& out, ValidationResult result);
}}

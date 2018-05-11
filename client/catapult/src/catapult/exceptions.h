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
#include "utils/BaseValue.h"
#include "utils/Logging.h"
#include "utils/NonCopyable.h"
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/throw_exception.hpp>
#include <atomic>
#include <exception>

#if defined(WIN32) || defined(WIN64)
#define VISIBLE_EXCEPTION_ATTRIBUTE
#else
#define VISIBLE_EXCEPTION_ATTRIBUTE __attribute__ ((visibility ("default")))
#endif

namespace catapult {

	/// Base class for all catapult exceptions that derives from both std::exception and boost::exception.
	template<typename TStlException>
	class VISIBLE_EXCEPTION_ATTRIBUTE catapult_error : public TStlException, public boost::exception, public utils::NonCopyable {
	public:
		/// Creates an exception with a message (\a what).
		explicit catapult_error(const char* what) : TStlException(what)
		{}

		/// Copy constructs an exception from \a rhs.
		catapult_error(const catapult_error& rhs)
				: TStlException(rhs.what())
				, boost::exception(rhs)
				, utils::NonCopyable()
		{}

		/// Move constructs an exception from \a rhs.
		catapult_error(catapult_error&& rhs)
				: TStlException(rhs.what())
				, boost::exception(std::move(rhs))
				, utils::NonCopyable()
		{}
	};

	template<typename TStlException>
	class VISIBLE_EXCEPTION_ATTRIBUTE catapult_error<catapult_error<TStlException>> : public catapult_error<TStlException> {
	public:
		/// Creates an exception with a message (\a what).
		explicit catapult_error(const char* what) : catapult_error<TStlException>(what)
		{}

		/// Copy constructs an exception from \a rhs.
		catapult_error(const catapult_error& rhs) : catapult_error<TStlException>(rhs)
		{}

		/// Move constructs an exception from \a rhs.
		catapult_error(catapult_error&& rhs) : catapult_error<TStlException>(std::move(rhs))
		{}

		/// Copy constructs an exception from \a rhs.
		catapult_error(const catapult_error<TStlException>& rhs) : catapult_error<TStlException>(rhs)
		{}

		/// Move constructs an exception from \a rhs.
		catapult_error(catapult_error<TStlException>&& rhs) : catapult_error<TStlException>(std::move(rhs))
		{}
	};

	using catapult_runtime_error = catapult_error<std::runtime_error>;
	using catapult_invalid_argument = catapult_error<std::invalid_argument>;
	using catapult_out_of_range = catapult_error<std::out_of_range>;
	using catapult_file_io_error = catapult_error<catapult_runtime_error>;

	/// Tag for first custom exception parameter.
	struct ErrorParam1 {};

	/// Tag for second custom exception parameter.
	struct ErrorParam2 {};

	namespace exception_detail {
		/// Converts \a value to a raw value.
		/// \note pointer types are not supported.
		template<typename T, typename X = typename std::enable_if<!std::is_pointer<T>::value>::type>
		constexpr T ConvertToValue(const T& value) {
			return value;
		}

		/// Converts \a value to a raw value.
		template<typename TValue, typename TTag>
		constexpr TValue ConvertToValue(const utils::BaseValue<TValue, TTag>& value) {
			return value.unwrap();
		}

		/// Converts \a value to a raw value.
		template<typename T>
		constexpr T ConvertToValue(const std::atomic<T>& value) {
			return value;
		}

		/// Helper class for creating boost::error_info.
		template<typename TErrorParam>
		class Make {
		public:
			/// Creates a boost::error_info with the specified \a value.
			template<typename T>
			static auto From(const T& value) {
				return boost::error_info<TErrorParam, decltype(ConvertToValue(value))>(ConvertToValue(value));
			}
		};
	}
}

/// Macro used to throw a catapult exception.
#define CATAPULT_THROW_EXCEPTION(EXCEPTION) \
	BOOST_THROW_EXCEPTION(EXCEPTION)

/// Macro used to throw a catapult exception with zero parameters.
#define CATAPULT_THROW_AND_LOG_0(TYPE, MESSAGE) { \
	CATAPULT_LOG(error) << "Throwing exception: " << MESSAGE; \
	CATAPULT_THROW_EXCEPTION(TYPE(MESSAGE)); \
}

/// Macro used to throw a catapult exception with one parameter.
#define CATAPULT_THROW_AND_LOG_1(TYPE, MESSAGE, PARAM1) { \
	auto detail1 = exception_detail::Make<ErrorParam1>::From(PARAM1); \
	CATAPULT_LOG(error) << "Throwing exception: " << MESSAGE << " (" << detail1.value() << ")"; \
	CATAPULT_THROW_EXCEPTION(TYPE(MESSAGE) << detail1); \
}

/// Macro used to throw a catapult exception with two parameters.
#define CATAPULT_THROW_AND_LOG_2(TYPE, MESSAGE, PARAM1, PARAM2) { \
	auto detail1 = exception_detail::Make<ErrorParam1>::From(PARAM1); \
	auto detail2 = exception_detail::Make<ErrorParam2>::From(PARAM2); \
	CATAPULT_LOG(error) << "Throwing exception: " << MESSAGE << " (" << detail1.value() << ", " << detail2.value() << ")"; \
	CATAPULT_THROW_EXCEPTION(TYPE(MESSAGE) << detail1 << detail2); \
}

/// Macro used to throw a catapult runtime error.
#define CATAPULT_THROW_RUNTIME_ERROR(MESSAGE) \
	CATAPULT_THROW_AND_LOG_0(catapult_runtime_error, MESSAGE)

/// Macro used to throw a catapult runtime error with a single parameter.
#define CATAPULT_THROW_RUNTIME_ERROR_1(MESSAGE, PARAM1) \
	CATAPULT_THROW_AND_LOG_1(catapult_runtime_error, MESSAGE, PARAM1)

/// Macro used to throw a catapult runtime error with two parameters.
#define CATAPULT_THROW_RUNTIME_ERROR_2(MESSAGE, PARAM1, PARAM2) \
	CATAPULT_THROW_AND_LOG_2(catapult_runtime_error, MESSAGE, PARAM1, PARAM2)

/// Macro used to throw a catapult invalid argument.
#define CATAPULT_THROW_INVALID_ARGUMENT(MESSAGE) \
	CATAPULT_THROW_AND_LOG_0(catapult_invalid_argument, MESSAGE)

/// Macro used to throw a catapult invalid argument with a single parameter.
#define CATAPULT_THROW_INVALID_ARGUMENT_1(MESSAGE, PARAM1) \
	CATAPULT_THROW_AND_LOG_1(catapult_invalid_argument, MESSAGE, PARAM1)

/// Macro used to throw a catapult invalid argument with two parameters.
#define CATAPULT_THROW_INVALID_ARGUMENT_2(MESSAGE, PARAM1, PARAM2) \
	CATAPULT_THROW_AND_LOG_2(catapult_invalid_argument, MESSAGE, PARAM1, PARAM2)

/// Macro used to throw a catapult out of range.
#define CATAPULT_THROW_OUT_OF_RANGE(MESSAGE) \
	CATAPULT_THROW_AND_LOG_0(catapult_out_of_range, MESSAGE)

/// Macro used to throw a catapult file io error.
#define CATAPULT_THROW_FILE_IO_ERROR(MESSAGE) \
	CATAPULT_THROW_AND_LOG_0(catapult_file_io_error, MESSAGE)

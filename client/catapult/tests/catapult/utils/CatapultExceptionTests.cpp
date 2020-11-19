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

#include "catapult/exceptions.h"
#include "tests/TestHarness.h"
#include <boost/exception/diagnostic_information.hpp>
#include <sstream>
#include <vector>

namespace catapult {

#define TEST_CLASS CatapultExceptionTests

	namespace {
		struct CustomTestTag1 {};
		using custom_info1 = boost::error_info<CustomTestTag1, int>;

		struct CustomTestTag2 {};
		using custom_info2 = boost::error_info<CustomTestTag2, int>;

		std::vector<std::string> GetLocationIndependentDiagnosticInformation(const boost::exception& ex) {
			// Arrange: remove file path and line number
			auto info = boost::diagnostic_information(ex);
			info = info.substr(info.find(':', 2) + 2);

			// - split the info by line
			std::vector<std::string> infoLines;
			size_t startPos = 0;
			size_t newLinePos = 0;
			while ((newLinePos = info.find('\n', startPos)) != std::string::npos) {
				infoLines.push_back(info.substr(startPos, newLinePos - startPos));
				startPos = newLinePos + 1;

				CATAPULT_LOG(trace) << infoLines.size() << ": " << infoLines.back();
			}

			return infoLines;
		}

		using ExpectedTagPairs = std::vector<std::pair<std::string, std::string>>;

		template<typename TTraits>
		struct ExpectedDiagnostics {
		public:
			std::string What;
			std::string FunctionName;
			ExpectedTagPairs TagPairs;
		};

#ifdef _MSC_VER
#define CUSTOM_TAG_NAMESPACE "catapult::`anonymous namespace'::"
#define CLASSPREFIX "class "
#define STRUCTPREFIX "struct "
#define PTRSUFFIX " * __ptr64"
#else
#define CUSTOM_TAG_NAMESPACE "catapult::(anonymous namespace)::"
#define CLASSPREFIX ""
#define STRUCTPREFIX ""
#define PTRSUFFIX "*"
#endif

		template<typename TException, typename TTraits>
		void AssertExceptionInformation(const TException& ex, const ExpectedDiagnostics<TTraits>& expected) {
			// Arrange:
			std::vector<std::string> expectedDiagLines{
				"Throw in function " + expected.FunctionName,
				"Dynamic exception type: " CLASSPREFIX "boost::wrapexcept<" + std::string(TTraits::Exception_Fqn) + " >",
				"std::exception::what: " + expected.What
			};

			std::set<std::string> expectedTagLines;
			for (const auto& pair : expected.TagPairs)
				expectedTagLines.insert("[" STRUCTPREFIX + pair.first + PTRSUFFIX "] = " + pair.second);

			auto diagLines = GetLocationIndependentDiagnosticInformation(ex);

			// Assert:
			EXPECT_EQ(expected.What, ex.what());

			// - check boost non-tag diagnostics
			EXPECT_EQ(3 + expected.TagPairs.size(), diagLines.size());
			for (auto i = 0u; i < expectedDiagLines.size(); ++i)
				EXPECT_EQ(expectedDiagLines[i], diagLines[i]) << "line " << i;

			// - check tag diagnostics
			for (auto i = expectedDiagLines.size(); i < diagLines.size(); ++i) {
				const auto& actualTagLine = diagLines[i];
				EXPECT_CONTAINS(expectedTagLines, actualTagLine);
			}
		}
	}

	namespace {
		struct RuntimeErrorTraits {
			using ExceptionType = catapult_runtime_error;
			static constexpr auto Exception_Fqn = CLASSPREFIX "catapult::catapult_error<" CLASSPREFIX "std::runtime_error>";
		};

		struct InvalidArgumentTraits {
			using ExceptionType = catapult_invalid_argument;
			static constexpr auto Exception_Fqn = CLASSPREFIX "catapult::catapult_error<" CLASSPREFIX "std::invalid_argument>";
		};

		struct OutOfRangeTraits {
			using ExceptionType = catapult_out_of_range;
			static constexpr auto Exception_Fqn = CLASSPREFIX "catapult::catapult_error<" CLASSPREFIX "std::out_of_range>";
		};

		struct FileIoErrorTraits {
			using ExceptionType = catapult_file_io_error;
			using BaseExceptionType = catapult_runtime_error;
			static constexpr auto Exception_Fqn =
				CLASSPREFIX "catapult::catapult_error<" CLASSPREFIX "catapult::catapult_error<" CLASSPREFIX "std::runtime_error> >";
		};
	}

	// region exception hierarchy tests

	TEST(TEST_CLASS, ExceptionHierarchyIsCorrect) {
		// Act + Assert: first level exceptions
		EXPECT_THROW(throw catapult_runtime_error("error"), std::runtime_error);
		EXPECT_THROW(throw catapult_invalid_argument("error"), std::invalid_argument);
		EXPECT_THROW(throw catapult_out_of_range("error"), std::out_of_range);

		// - second level exceptions
		EXPECT_THROW(throw catapult_file_io_error("error"), catapult_runtime_error);
		EXPECT_THROW(throw catapult_file_io_error("error"), std::runtime_error);
	}

	// endregion

	// region basic exception tests

#define EXCEPTION_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_RuntimeError) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RuntimeErrorTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_InvalidArgument) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<InvalidArgumentTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_OutOfRange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<OutOfRangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_FileIoErrorTraits) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<FileIoErrorTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	EXCEPTION_TRAITS_BASED_TEST(ExceptionDerivesFromBasicExceptionTypes) {
		EXPECT_THROW(throw typename TTraits::ExceptionType("error"), std::exception);
		EXPECT_THROW(throw typename TTraits::ExceptionType("error"), boost::exception);
	}

	EXCEPTION_TRAITS_BASED_TEST(CanThrowExceptionWithCustomMessage) {
		try {
			// Act:
			CATAPULT_THROW_EXCEPTION(typename TTraits::ExceptionType("custom error message"));
		} catch (const typename TTraits::ExceptionType& ex) {
			// Assert:
			ExpectedDiagnostics<TTraits> expected;
			expected.What = "custom error message";
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			AssertExceptionInformation(ex, expected);
		}
	}

	namespace {
		template<typename TException>
		TException CreateExceptionWithCustomMessageAndInfo() {
			return TException("custom error message") << custom_info1(17) << custom_info2(3);
		}

		template<typename TTraits>
		ExpectedDiagnostics<TTraits> CreateExpectedDiagnosticsForExceptionWithCustomMessageAndInfo() {
			ExpectedDiagnostics<TTraits> expected;
			expected.What = "custom error message";
			expected.TagPairs.push_back(std::make_pair(CUSTOM_TAG_NAMESPACE "CustomTestTag1", "17"));
			expected.TagPairs.push_back(std::make_pair(CUSTOM_TAG_NAMESPACE "CustomTestTag2", "3"));
			return expected;
		}
	}

	EXCEPTION_TRAITS_BASED_TEST(CanThrowExceptionWithCustomMessageAndCustomPodPayloads) {
		try {
			// Act:
			CATAPULT_THROW_EXCEPTION(CreateExceptionWithCustomMessageAndInfo<typename TTraits::ExceptionType>());
		} catch (const typename TTraits::ExceptionType& ex) {
			// Assert:
			auto expected = CreateExpectedDiagnosticsForExceptionWithCustomMessageAndInfo<TTraits>();
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			AssertExceptionInformation(ex, expected);
		}
	}

	EXCEPTION_TRAITS_BASED_TEST(CanCopyConstructException) {
		try {
			// Act:
			auto ex = CreateExceptionWithCustomMessageAndInfo<typename TTraits::ExceptionType>();
			auto ex2 = ex;
			CATAPULT_THROW_EXCEPTION(ex2);
		} catch (const typename TTraits::ExceptionType& ex) {
			// Assert:
			auto expected = CreateExpectedDiagnosticsForExceptionWithCustomMessageAndInfo<TTraits>();
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			AssertExceptionInformation(ex, expected);
		}
	}

	EXCEPTION_TRAITS_BASED_TEST(CanMoveConstructException) {
		try {
			// Act:
			auto ex = CreateExceptionWithCustomMessageAndInfo<typename TTraits::ExceptionType>();
			auto ex2 = std::move(ex);
			CATAPULT_THROW_EXCEPTION(ex2);
		} catch (const typename TTraits::ExceptionType& ex) {
			// Assert:
			auto expected = CreateExpectedDiagnosticsForExceptionWithCustomMessageAndInfo<TTraits>();
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			AssertExceptionInformation(ex, expected);
		}
	}

	// endregion

	// region basic exception copy by value tests

	EXCEPTION_TRAITS_BASED_TEST(CanThrowExceptionWithCustomMessageAndCustomStringPayload) {
		try {
			// Act:
			auto customInfo = exception_detail::Make<ErrorParam1>::From(std::string("string info"));
			CATAPULT_THROW_EXCEPTION(typename TTraits::ExceptionType("custom error message") << customInfo);
		} catch (const typename TTraits::ExceptionType& ex) {
			// Assert:
			ExpectedDiagnostics<TTraits> expected;
			expected.What = "custom error message";
			expected.TagPairs.push_back(std::make_pair("catapult::ErrorParam1", "string info"));
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			AssertExceptionInformation(ex, expected);
		}
	}

	EXCEPTION_TRAITS_BASED_TEST(CanThrowExceptionWithCustomMessageAndCustomHexFormattedPayload) {
		try {
			// Act:
			std::vector<uint8_t> buffer{ 0x68, 0x65, 0x6C, 0x6C, 0x6F };
			auto customInfo = exception_detail::Make<ErrorParam1>::From(utils::HexFormat(buffer));
			CATAPULT_THROW_EXCEPTION(typename TTraits::ExceptionType("custom error message") << customInfo);
		} catch (const typename TTraits::ExceptionType& ex) {
			// Assert:
			ExpectedDiagnostics<TTraits> expected;
			expected.What = "custom error message";
			expected.TagPairs.push_back(std::make_pair("catapult::ErrorParam1", "68656C6C6F"));
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			AssertExceptionInformation(ex, expected);
		}
	}

	// endregion

	// region derived exception tests

#define DERIVED_EXCEPTION_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_FileIoErrorTraits) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<FileIoErrorTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DERIVED_EXCEPTION_TRAITS_BASED_TEST(CanCopyConstructDerivedExceptionFromBaseException) {
		try {
			// Act:
			auto ex = CreateExceptionWithCustomMessageAndInfo<typename TTraits::BaseExceptionType>();
			typename TTraits::ExceptionType ex2 = ex;
			CATAPULT_THROW_EXCEPTION(ex2);
		} catch (const typename TTraits::ExceptionType& ex) {
			// Assert:
			auto expected = CreateExpectedDiagnosticsForExceptionWithCustomMessageAndInfo<TTraits>();
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			AssertExceptionInformation(ex, expected);
		}
	}

	DERIVED_EXCEPTION_TRAITS_BASED_TEST(CanMoveConstructDerivedExceptionFromBaseException) {
		try {
			// Act:
			auto ex = CreateExceptionWithCustomMessageAndInfo<typename TTraits::BaseExceptionType>();
			typename TTraits::ExceptionType ex2 = std::move(ex);
			CATAPULT_THROW_EXCEPTION(ex2);
		} catch (const typename TTraits::ExceptionType& ex) {
			// Assert:
			auto expected = CreateExpectedDiagnosticsForExceptionWithCustomMessageAndInfo<TTraits>();
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			AssertExceptionInformation(ex, expected);
		}
	}

	// endregion

	// region macro tests

#define ASSERT_THROW_MACRO_0(CATAPULT_THROW, TRAITS) \
	do { \
		try { \
			CATAPULT_THROW(#TRAITS); \
		} catch (const TRAITS::ExceptionType& ex) { \
			ExpectedDiagnostics<TRAITS> expected; \
			expected.What = #TRAITS; \
			expected.FunctionName = BOOST_CURRENT_FUNCTION; \
			AssertExceptionInformation(ex, expected); \
		} \
	} while (false)

	TEST(TEST_CLASS, CanThrowCustomMessageUsingExceptionMacro) {
		ASSERT_THROW_MACRO_0(CATAPULT_THROW_RUNTIME_ERROR, RuntimeErrorTraits);
		ASSERT_THROW_MACRO_0(CATAPULT_THROW_INVALID_ARGUMENT, InvalidArgumentTraits);
		ASSERT_THROW_MACRO_0(CATAPULT_THROW_OUT_OF_RANGE, OutOfRangeTraits);
		ASSERT_THROW_MACRO_0(CATAPULT_THROW_FILE_IO_ERROR, FileIoErrorTraits);
	}

#define ASSERT_THROW_MACRO_1(CATAPULT_THROW, TRAITS) \
	do { \
		try { \
			CATAPULT_THROW(#TRAITS, 12); \
		} catch (const TRAITS::ExceptionType& ex) { \
			ExpectedDiagnostics<TRAITS> expected; \
			expected.What = #TRAITS; \
			expected.FunctionName = BOOST_CURRENT_FUNCTION; \
			expected.TagPairs.push_back(std::make_pair("catapult::ErrorParam1", "12")); \
			AssertExceptionInformation(ex, expected); \
		} \
	} while (false)

	TEST(TEST_CLASS, CanThrowCustomMessageWithOneParamterUsingExceptionMacro) {
		ASSERT_THROW_MACRO_1(CATAPULT_THROW_RUNTIME_ERROR_1, RuntimeErrorTraits);
		ASSERT_THROW_MACRO_1(CATAPULT_THROW_INVALID_ARGUMENT_1, InvalidArgumentTraits);
	}

#define ASSERT_THROW_MACRO_2(CATAPULT_THROW, TRAITS) \
	do { \
		try { \
			CATAPULT_THROW(#TRAITS, 12, 27); \
		} catch (const TRAITS::ExceptionType& ex) { \
			ExpectedDiagnostics<TRAITS> expected; \
			expected.What = #TRAITS; \
			expected.FunctionName = BOOST_CURRENT_FUNCTION; \
			expected.TagPairs.push_back(std::make_pair("catapult::ErrorParam1", "12")); \
			expected.TagPairs.push_back(std::make_pair("catapult::ErrorParam2", "27")); \
			AssertExceptionInformation(ex, expected); \
		} \
	} while (false)

	TEST(TEST_CLASS, CanThrowCustomMessageWithTwoParamtersUsingExceptionMacro) {
		ASSERT_THROW_MACRO_2(CATAPULT_THROW_RUNTIME_ERROR_2, RuntimeErrorTraits);
		ASSERT_THROW_MACRO_2(CATAPULT_THROW_INVALID_ARGUMENT_2, InvalidArgumentTraits);
	}

	// endregion

	// region other exception tests

	TEST(TEST_CLASS, CanRethrowWithAdditionalInfo) {
		try {
			try {
				// Act: throw an exception
				CATAPULT_THROW_EXCEPTION(catapult_runtime_error("original") << custom_info1(12));
			} catch (catapult_runtime_error& ex) {
				// - add additional info and rethrow
				ex << custom_info2(45);
				throw;
			}
		} catch (const catapult_runtime_error& ex) {
			// Assert:
			ExpectedDiagnostics<RuntimeErrorTraits> expected;
			expected.What = "original";
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			expected.TagPairs.push_back(std::make_pair(CUSTOM_TAG_NAMESPACE "CustomTestTag1", "12"));
			expected.TagPairs.push_back(std::make_pair(CUSTOM_TAG_NAMESPACE "CustomTestTag2", "45"));
			AssertExceptionInformation(ex, expected);
		}
	}

	TEST(TEST_CLASS, CanRethrowBaseExceptionAsMoreSpecificException) {
		try {
			try {
				// Act: throw an exception
				CATAPULT_THROW_EXCEPTION(catapult_runtime_error("original") << custom_info1(12));
			} catch (const catapult_runtime_error& ex) {
				// - make a copy, add additional info, rethrow
				catapult_file_io_error ex2(ex);
				ex2 << custom_info2(45);
				CATAPULT_THROW_EXCEPTION(ex2);
			}
		} catch (const catapult_runtime_error& ex) {
			// Assert:
			ExpectedDiagnostics<FileIoErrorTraits> expected;
			expected.What = "original";
			expected.FunctionName = BOOST_CURRENT_FUNCTION;
			expected.TagPairs.push_back(std::make_pair(CUSTOM_TAG_NAMESPACE "CustomTestTag1", "12"));
			expected.TagPairs.push_back(std::make_pair(CUSTOM_TAG_NAMESPACE "CustomTestTag2", "45"));
			AssertExceptionInformation(ex, expected);
		}
	}

	// endregion

	// region exception_detail tests

	namespace {
		struct PodTagTraits {
			template<typename T>
			using Type = T;
		};

		struct AtomicTagTraits {
			template<typename T>
			using Type = typename std::atomic<T>;
		};

		struct BaseValueTagTraits {
			template<typename T>
			using Type = typename utils::BaseValue<T, CustomTestTag1>;
		};
	}

#define TAG_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Pod) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PodTagTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Atomic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AtomicTagTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BaseValue) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BaseValueTagTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TAG_TRAITS_BASED_TEST(ConvertToValueSupports) {
		EXPECT_EQ(123, exception_detail::ConvertToValue(typename TTraits::template Type<int>(123)));
		EXPECT_EQ(8u, exception_detail::ConvertToValue(typename TTraits::template Type<unsigned int>(8)));
		EXPECT_EQ('h', exception_detail::ConvertToValue(typename TTraits::template Type<char>('h')));
	}

	TAG_TRAITS_BASED_TEST(CanMakeErrorInfoFrom) {
		// Arrange:
		using MakeCustomTestTag1 = exception_detail::Make<CustomTestTag1>;

		// Act + Assert:
		EXPECT_EQ(123, MakeCustomTestTag1::From(typename TTraits::template Type<int>(123)).value());
		EXPECT_EQ(8u, MakeCustomTestTag1::From(typename TTraits::template Type<unsigned int>(8)).value());
		EXPECT_EQ('h', MakeCustomTestTag1::From(typename TTraits::template Type<char>('h')).value());
	}

	TEST(TEST_CLASS, CanMakeErrorInfoFrom_String) {
		// Arrange:
		using MakeCustomTestTag1 = exception_detail::Make<CustomTestTag1>;
		using ErrorInfoType = decltype(MakeCustomTestTag1::From(std::string()));

		// Act: force payload to go out of scope
		std::unique_ptr<ErrorInfoType> pErrorInfo;
		{
			pErrorInfo = std::make_unique<ErrorInfoType>(MakeCustomTestTag1::From(std::string("a string")));
		}

		// Assert: error info has valid string even though original data has been destroyed
		EXPECT_EQ("a string", pErrorInfo->value());
	}

	TEST(TEST_CLASS, CanMakeErrorInfoFrom_HexContainer) {
		// Arrange:
		using MakeCustomTestTag1 = exception_detail::Make<CustomTestTag1>;
		using ErrorInfoType = decltype(MakeCustomTestTag1::From(std::string()));

		// Act: force payload to go out of scope
		std::unique_ptr<ErrorInfoType> pErrorInfo;
		{
			std::vector<uint8_t> buffer{ 0xA4, 0xB5, 0x8C, 0x9A, 0xED };
			pErrorInfo = std::make_unique<ErrorInfoType>(MakeCustomTestTag1::From(utils::HexFormat(buffer)));
		}

		// Assert: error info has valid string even though original data has been destroyed
		EXPECT_EQ("A4B58C9AED", pErrorInfo->value());
	}

	// endregion
}

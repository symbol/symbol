#pragma once
#include "catapult/utils/Logging.h"
#include <boost/exception/diagnostic_information.hpp>
#include <sstream>
#include <gtest/gtest.h>

namespace catapult { namespace test {

// if STRESS is defined to an iteration number, every test will be repeated that number of times
// for example, the following code will cause all tests to run 100 times
// #define STRESS 100u
// #include "tests/test/Stress.h"
#ifdef STRESS
#undef TEST

#define TEST_FRIENDLY_NAME_(TEST_FIXTURE, TEST_NAME) #TEST_FIXTURE "::" #TEST_NAME
#define TEST_IMPL_NAME_(TEST_FIXTURE, TEST_NAME) TEST_FIXTURE##_##TEST_NAME##_Impl

#define TEST(TEST_FIXTURE, TEST_NAME) \
	void TEST_IMPL_NAME_(TEST_FIXTURE, TEST_NAME)(); \
	\
	GTEST_TEST(TEST_FIXTURE, TEST_NAME) { \
		for (auto stressCounter = 1u; stressCounter <= STRESS; ++stressCounter) { \
			const auto Test_Fqn = TEST_FRIENDLY_NAME_(TEST_FIXTURE, TEST_NAME); \
			CATAPULT_LOG(debug) << "---- iter " << stressCounter << "/" << STRESS << " " \
				<< Test_Fqn << " ----"; \
			\
			try { \
				TEST_IMPL_NAME_(TEST_FIXTURE, TEST_NAME)(); \
			} catch (...) { \
				/* flatten error information into std::runtime_error for better jenkins reporting */ \
				std::stringstream exceptionMessage; \
				exceptionMessage << "Unhandled exception during " \
					<< Test_Fqn << " iteration " << stressCounter << "!" << std::endl \
					<< boost::current_exception_diagnostic_information(); \
				CATAPULT_LOG(fatal) << exceptionMessage.str(); \
				throw std::runtime_error(exceptionMessage.str().c_str()); \
			} \
			\
			if (!::testing::Test::HasFailure()) continue; \
			CATAPULT_LOG(error) << Test_Fqn << " failed on iteration " << stressCounter; \
			return; \
		} \
	} \
	\
	void TEST_IMPL_NAME_(TEST_FIXTURE, TEST_NAME)()
#endif

// NO_STRESS_TEST should be used by tests that shouldn't be stressed
#define NO_STRESS_TEST(TEST_FIXTURE, TEST_NAME) GTEST_TEST(TEST_FIXTURE, TEST_NAME)
}}

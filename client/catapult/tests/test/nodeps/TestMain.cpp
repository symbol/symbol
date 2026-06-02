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

#include "src/catapult/crypto/OpensslInit.h"
#include "src/catapult/utils/ConfigurationValueParsers.h"
#include "src/catapult/utils/Logging.h"
#include "src/catapult/version/version.h"
#include "src/catapult/preprocessor.h"
#include "tests/TestHarness.h"
#include <thread>
#include <exception>
#include <iostream>
#include <typeinfo>
#include <boost/exception/diagnostic_information.hpp>
#ifdef _MSC_VER
#include <crtdbg.h>
#include <stdlib.h>
#include <windows.h>
#include <dbghelp.h>
#include <mutex>
#pragma comment(lib, "dbghelp.lib")
#endif

#ifdef CATAPULT_DOCKER_TESTS
extern int global_argc;
extern char** global_argv;
int global_argc;
char** global_argv;
#endif

namespace catapult { namespace test {

	namespace {
		uint32_t global_stress_iteration_count = 0;
	}

	uint32_t GetStressIterationCount() {
		return global_stress_iteration_count;
	}

	unsigned short GetLocalHostPort() {
		return GetStressIterationCount() ? 3014 : 2014;
	}

	uint32_t GetMaxNonDeterministicTestRetries() {
		return GetStressIterationCount() ? 500 : 25;
	}

	uint32_t GetNumDefaultPoolThreads() {
		return std::max<uint32_t>(16, 2 * std::thread::hardware_concurrency());
	}

	namespace {
		std::shared_ptr<void> SetupLogging() {
			utils::BasicLoggerOptions options;
			options.SinkType = utils::LogSinkType::Sync;
#ifndef _MSC_VER
			options.ColorMode = utils::LogColorMode::Ansi;
#endif

			auto pBootstrapper = std::make_shared<utils::LoggingBootstrapper>();
			pBootstrapper->addConsoleLogger(options, utils::LogFilter(utils::LogLevel::debug));
			return PORTABLE_MOVE(pBootstrapper);
		}

		uint32_t GetArgumentUint32(const std::string& name, int argc, char** argv) {
			auto key = "--cat_" + name + "=";
			for (auto i = 0; i < argc; ++i) {
				auto argumentKeyValue = std::string(argv[i]);
				if (0 != argumentKeyValue.find(key))
					continue;

				uint32_t parsedValue;
				auto value = argumentKeyValue.substr(key.size());
				if (!utils::TryParseValue(value, parsedValue))
					CATAPULT_LOG(warning) << "argument '" << name << "' has invalid value: " << value;

				return parsedValue;
			}

			return 0;
		}
	}

#ifdef _MSC_VER
	// TEMP DIAGNOSTIC: print a symbolized stack trace of the current thread to stderr
	void PrintDiagnosticStackTrace(const char* tag, const char* message) {
		static std::mutex s_traceMutex;
		std::lock_guard<std::mutex> guard(s_traceMutex);

		std::cerr << "=== DIAGNOSTIC STACK TRACE (" << tag << ") ===" << std::endl;
		if (message)
			std::cerr << "message: " << message;

		void* frames[64];
		auto frameCount = ::CaptureStackBackTrace(0, 64, frames, nullptr);

		auto process = ::GetCurrentProcess();
		alignas(SYMBOL_INFO) char symbolBuffer[sizeof(SYMBOL_INFO) + 256 * sizeof(char)] = {};
		auto* pSymbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = 255;

		for (USHORT i = 0; i < frameCount; ++i) {
			auto address = reinterpret_cast<DWORD64>(frames[i]);
			std::cerr << "  #" << i << " " << frames[i];
			if (::SymFromAddr(process, address, nullptr, pSymbol))
				std::cerr << " " << pSymbol->Name;

			IMAGEHLP_LINE64 line = {};
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			DWORD displacement = 0;
			if (::SymGetLineFromAddr64(process, address, &displacement, &line))
				std::cerr << " (" << line.FileName << ":" << line.LineNumber << ")";

			std::cerr << std::endl;
		}

		std::cerr.flush();
	}
#endif
}}

int main(int argc, char** argv) {
	catapult::version::WriteVersionInformation(std::cout);
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

#ifdef _MSC_VER
	// TEMP DIAGNOSTIC: capture an unhandled CRT debug assert (e.g. iterator-debug) with a stack trace.
	// The report hook fires inline at the assert site (stack intact); print message + stack, then exit
	// deterministically so we neither hang on a dialog nor lose the location.
	::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
	::SymInitialize(::GetCurrentProcess(), nullptr, TRUE);

	// STL iterator-debug assertions are reported through the WIDE _CrtDbgReportW, which only invokes a
	// hook installed via _CrtSetReportHookW2; install both wide and ANSI hooks to be safe.
	_CrtSetReportHookW2(_CRT_RPTHOOK_INSTALL, [](int reportType, wchar_t* message, int* pReturnValue) -> int {
		if (_CRT_ASSERT == reportType || _CRT_ERROR == reportType) {
			if (message)
				std::wcerr << L"assert message: " << message << std::endl;

			catapult::test::PrintDiagnosticStackTrace(_CRT_ASSERT == reportType ? "CRT_ASSERT(W)" : "CRT_ERROR(W)", nullptr);
			std::cerr.flush();
			::fflush(stderr);
			_exit(42);
		}

		if (pReturnValue)
			*pReturnValue = 0;
		return FALSE;
	});

	_CrtSetReportHook([](int reportType, char* message, int* pReturnValue) -> int {
		if (_CRT_ASSERT == reportType || _CRT_ERROR == reportType) {
			catapult::test::PrintDiagnosticStackTrace(_CRT_ASSERT == reportType ? "CRT_ASSERT" : "CRT_ERROR", message);
			std::cerr.flush();
			::fflush(stderr);
			_exit(42);
		}

		if (pReturnValue)
			*pReturnValue = 0;
		return TRUE;
	});
#endif

	// TEMP DIAGNOSTIC: print the in-flight exception (with stack) when an uncaught exception escapes a worker thread
	std::set_terminate([]() {
		std::cerr << "=== TERMINATE HANDLER INVOKED ===" << std::endl;
		auto exceptionPtr = std::current_exception();
		if (exceptionPtr) {
			try {
				std::rethrow_exception(exceptionPtr);
			} catch (const std::exception& ex) {
				std::cerr << "unhandled std::exception [" << typeid(ex).name() << "]: " << ex.what() << std::endl;
				std::cerr << "boost diagnostic: " << boost::diagnostic_information(ex) << std::endl;
			} catch (...) {
				std::cerr << "unhandled non-std exception" << std::endl;
			}
		} else {
			std::cerr << "terminate called with no active exception" << std::endl;
		}

#ifdef _MSC_VER
		catapult::test::PrintDiagnosticStackTrace("TERMINATE", nullptr);
#endif
		std::cerr.flush();
		std::abort();
	});

	std::cout << "Initializing Logging..." << std::endl;
	auto pLoggingGuard = catapult::test::SetupLogging();

	std::cout << "Initializing and Running Tests..." << std::endl;
	::testing::InitGoogleTest(&argc, argv);

	std::cout << "Initializing OpenSSL crypto functions" << std::endl;
	auto pOpensslContext = catapult::crypto::SetupOpensslCryptoFunctions();

#ifdef CATAPULT_DOCKER_TESTS
	global_argc = argc;
	global_argv = argv;
#endif

	if (argc >= 2) {
		auto& count = catapult::test::global_stress_iteration_count;
		count = catapult::test::GetArgumentUint32("stress", argc, argv);
		CATAPULT_LOG(warning) << "Note: Catapult Test stress iteration count = " << count << std::endl;
	}

	return RUN_ALL_TESTS();
}

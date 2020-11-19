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

#include "catapult/local/server/MemoryCounters.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS MemoryCountersTests

	namespace {
		using Counters = std::vector<utils::DiagnosticCounter>;

		bool HasName(const utils::DiagnosticCounter& counter, const std::string& name) {
			return name == counter.id().name();
		}

		bool HasCounter(const Counters& counters, const std::string& name) {
			return std::any_of(counters.cbegin(), counters.cend(), [&name](const auto& counter) { return HasName(counter, name); });
		}

		uint64_t GetValue(const Counters& counters, const std::string& name) {
			for (const auto& counter : counters) {
				if (HasName(counter, name))
					return counter.value();
			}

			CATAPULT_THROW_INVALID_ARGUMENT_1("could not find counter with name", name);
		}

		constexpr size_t GetNumExpectedCounters() {
#ifdef __APPLE__
			return 3;
#else
			return 4;
#endif
		}
	}

	TEST(TEST_CLASS, CanAddMemoryCounters) {
		// Act:
		Counters counters;
		AddMemoryCounters(counters);

		// Assert:
		EXPECT_EQ(GetNumExpectedCounters(), counters.size());

		// - check common counters
		HasCounter(counters, "MEM CUR RSS");
		HasCounter(counters, "MEM MAX RSS");

		// - check platform dependent counters
#ifdef _WIN32
		HasCounter(counters, "MEM CUR CMT");
		HasCounter(counters, "MEM MAX CMT");
#else
		HasCounter(counters, "MEM CUR VIRT");
#if !defined(__APPLE__)
		HasCounter(counters, "MEM SHR RSS");
#endif
#endif
	}

	TEST(TEST_CLASS, CountersHaveNonzeroValues) {
		// Arrange:
		Counters counters;
		AddMemoryCounters(counters);

		// Act:
		for (const auto& counter : counters) {
			CATAPULT_LOG(debug) << counter.id().name() << " : " << counter.value();

			// Assert:
			EXPECT_NE(0u, counter.value()) << counter.id().name() << " has zero value";
		}
	}

	TEST(TEST_CLASS, CountersHaveExpectedRelationship) {
		// Arrange:
		Counters counters;
		AddMemoryCounters(counters);

		// Assert:
		EXPECT_GE(GetValue(counters, "MEM MAX RSS"), GetValue(counters, "MEM CUR RSS"));

#ifdef _WIN32
		EXPECT_GE(GetValue(counters, "MEM MAX CMT"), GetValue(counters, "MEM CUR CMT"));
#else
		EXPECT_GE(GetValue(counters, "MEM CUR VIRT"), GetValue(counters, "MEM CUR RSS"));
#if !defined(__APPLE__)
		EXPECT_GE(GetValue(counters, "MEM MAX RSS"), GetValue(counters, "MEM SHR RSS"));
#endif
#endif
	}
}}

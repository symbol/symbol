#include "catapult/local/MemoryCounters.h"
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

	TEST(TEST_CLASS, CountersHaveNonZeroValues) {
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

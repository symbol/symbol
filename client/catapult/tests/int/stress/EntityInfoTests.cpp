#include "catapult/model/EntityInfo.h"
#include "catapult/model/WeakEntityInfo.h"
#include "catapult/utils/StackLogger.h"
#include "tests/TestHarness.h"
#include <chrono>

namespace catapult { namespace model {

	namespace {
		constexpr auto Max_Copy_Penalty_Ratio = 6.5;

#ifdef STRESS
		constexpr auto Number_Of_Elements = 35'000'000u;
#else
		constexpr auto Number_Of_Elements = 350'000u;
#endif

		using IntInfo = model::EntityInfo<int>;
		using IntInfos = std::vector<IntInfo>;

		using IntWeakInfo = model::WeakEntityInfoT<int>;
		using IntWeakInfos = std::vector<IntWeakInfo>;

		Hash256 numberToHash(uint32_t number) {
			return {{
				static_cast<uint8_t>(number & 0xFF),
				static_cast<uint8_t>((number >> 8) & 0xFF),
				static_cast<uint8_t>((number >> 16) & 0xFF),
				static_cast<uint8_t>((number >> 24) & 0xFF)
			}};
		}

		IntInfos PrepareInfos(size_t count) {
			IntInfos infos;
			infos.reserve(count);

			for (auto i = 0u; i < count; ++i)
				infos.push_back(IntInfo(std::make_shared<int>(3 * i + 1), numberToHash(i)));

			return infos;
		}

		uint64_t MeasureTime(const std::function<void ()>& measuredFunction) {
			auto start = std::chrono::steady_clock::now();
			measuredFunction();
			auto elapsed = std::chrono::steady_clock::now() - start;
			return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
		}

		uint64_t MeasureEntityInfo(size_t count) {
			// Arrange:
			auto sourceInfos = PrepareInfos(count);
			IntInfos infos;

			// Act: (similar to CollectRevertedTransactionInfos)
			auto timeTaken = MeasureTime([&sourceInfos, &infos]() {
				for (auto& info : sourceInfos)
					infos.push_back(std::move(info));
			});

			// Assert:
			EXPECT_EQ(count, infos.size());
			return timeTaken;
		}

		uint64_t MeasureWeakEntityInfo(size_t count) {
			// Arrange:
			auto sourceInfos = PrepareInfos(count);
			IntWeakInfos infos;

			// Act:
			auto timeTaken = MeasureTime([&sourceInfos, &infos]() {
				for (const auto& info : sourceInfos)
					infos.push_back(IntWeakInfo(*info.pEntity, info.EntityHash));
			});

			// Assert:
			EXPECT_EQ(count, infos.size());
			return timeTaken;
		}
	}

	NO_STRESS_TEST(EntityInfoIntegrityTests, MovingSpeedIsNegligible) {
		// Act:
		auto copyEntityInfoSpeed = MeasureEntityInfo(Number_Of_Elements);
		CATAPULT_LOG(debug) << "copyEntityInfoSpeed: " << copyEntityInfoSpeed;

		auto copyWeakEntityInfoSpeed = MeasureWeakEntityInfo(Number_Of_Elements);
		CATAPULT_LOG(debug) << "copyWeakEntityInfoSpeed: " << copyWeakEntityInfoSpeed;

		auto copyPenaltyRatio = copyEntityInfoSpeed / (copyWeakEntityInfoSpeed * 1.0);
		CATAPULT_LOG(debug) << "copyEntityInfoSpeed / copyWeakEntityInfoSpeed: " << copyPenaltyRatio;

		// Assert:
		EXPECT_LE(copyPenaltyRatio, Max_Copy_Penalty_Ratio);
	}
}}

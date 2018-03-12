#include "sync/src/SchedulerService.h"
#include "sync/src/TasksConfiguration.h"
#include "catapult/thread/Scheduler.h"
#include "tests/test/core/SchedulerTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS SchedulerServiceTests

	namespace {
		constexpr auto Counter_Name = "TASKS";
		constexpr auto Service_Name = "scheduler";

		struct SchedulerServiceTraits {
			static auto CreateRegistrar(const TasksConfiguration& config) {
				return CreateSchedulerServiceRegistrar(config);
			}

			static auto CreateRegistrar() {
				return CreateRegistrar(TasksConfiguration::Uninitialized());
			}
		};

		using TestContext = test::ServiceLocatorTestContext<SchedulerServiceTraits>;

		thread::Task CreateTask(size_t id) {
			return thread::CreateNamedTask("task " + std::to_string(id), []() {
				return thread::make_ready_future(thread::TaskResult::Continue);
			});
		}

		TaskConfiguration CreateTaskConfiguration(uint32_t startDelayMs, uint32_t repeatDelayMs) {
			auto config = TaskConfiguration::Uninitialized();
			config.StartDelay = utils::TimeSpan::FromMilliseconds(startDelayMs);
			config.RepeatDelay = utils::TimeSpan::FromMilliseconds(repeatDelayMs);
			return config;
		}
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(Scheduler, Post_Tasks)

	// region basic boot and shutdown

	TEST(TEST_CLASS, CanBootSchedulerWithoutTasks) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(1u, context.locator().numServices());
		EXPECT_EQ(1u, context.locator().counters().size());

		EXPECT_TRUE(!!context.locator().service<thread::Scheduler>(Service_Name));
		EXPECT_EQ(0u, context.counter(Counter_Name));
	}

	TEST(TEST_CLASS, CanBootSchedulerWithTasks) {
		// Arrange:
		TestContext context;

		// - add tasks with corresponding config entries
		auto config = TasksConfiguration::Uninitialized();
		for (auto i = 0u; i < 3u; ++i) {
			auto task = CreateTask(i);
			context.testState().state().tasks().push_back(task);
			config.Tasks.emplace(task.Name, CreateTaskConfiguration(1, 1));
		}

		// Act:
		context.boot(config);

		// Assert:
		EXPECT_EQ(1u, context.locator().numServices());
		EXPECT_EQ(1u, context.locator().counters().size());

		EXPECT_TRUE(!!context.locator().service<thread::Scheduler>(Service_Name));
		EXPECT_EQ(3u, context.counter(Counter_Name));
	}

	TEST(TEST_CLASS, CanShutdownService) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();
		context.shutdown();

		// Assert:
		EXPECT_EQ(1u, context.locator().numServices());
		EXPECT_EQ(1u, context.locator().counters().size());

		EXPECT_FALSE(!!context.locator().service<thread::Scheduler>(Service_Name));
		EXPECT_EQ(static_cast<uint64_t>(extensions::ServiceLocator::Sentinel_Counter_Value), context.counter(Counter_Name));
	}

	// endregion

	// region task configuration

	TEST(TEST_CLASS, CannotBootSchedulerWithTaskWithoutCorrespondingConfiguration) {
		// Arrange:
		TestContext context;

		// - add task but don't add config entry
		context.testState().state().tasks().push_back(CreateTask(12));

		// Act + Assert:
		EXPECT_THROW(context.boot(), catapult_invalid_argument);
	}

	namespace {
		thread::Task CreateContinuousTaskWithCounter(const std::string& name, std::atomic<uint32_t>& counter) {
			return thread::CreateNamedTask(name, [&counter]() {
				++counter;
				return thread::make_ready_future(thread::TaskResult::Continue);
			});
		}
	}

#define EXPECT_EQ_RETRY(EXPECTED, ACTUAL) test::ExpectEqualOrRetry((EXPECTED), (ACTUAL), #EXPECTED, #ACTUAL)

	TEST(TEST_CLASS, SchedulerRespectsInitialDelayFromConfiguration) {
		// Assert: non-deterministic because delay is impacted by scheduling
		test::RunNonDeterministicTest("SchedulerService", [](auto i) {
			// Arrange:
			TestContext context;

			// - add a single task with a config
			auto timeUnit = test::GetTimeUnitForIteration(i);
			std::atomic<uint32_t> counter(0);
			context.testState().state().tasks().push_back(CreateContinuousTaskWithCounter("gamma", counter));

			auto config = TasksConfiguration::Uninitialized();
			config.Tasks.emplace("gamma", CreateTaskConfiguration(2 * timeUnit, 20 * timeUnit));

			// Act:
			context.boot(config);

			// Assert: after sleeping 0.5x the initial delay, no tasks should have run
			test::Sleep(timeUnit);
			if (!EXPECT_EQ_RETRY(0u, counter))
				return false;

			// Assert: after sleeping 1.5x the initial delay, one task should have run and
			//         the task should still be scheduled
			test::Sleep(2 * timeUnit);
			if (!EXPECT_EQ_RETRY(1u, counter))
				return false;

			EXPECT_EQ(1u, context.counter(Counter_Name));
			return true;
		});
	}

	TEST(TEST_CLASS, SchedulerRespectsRepeatDelayFromConfiguration) {
		// Assert: non-deterministic because delay is impacted by scheduling
		test::RunNonDeterministicTest("SchedulerService", [](auto i) {
			// Arrange:
			TestContext context;

			// - add a single task with a config
			auto timeUnit = test::GetTimeUnitForIteration(i);
			std::atomic<uint32_t> counter(0);
			context.testState().state().tasks().push_back(CreateContinuousTaskWithCounter("gamma", counter));

			auto config = TasksConfiguration::Uninitialized();
			config.Tasks.emplace("gamma", CreateTaskConfiguration(timeUnit, 2 * timeUnit));

			// Act:
			context.boot(config);

			// Assert: after sleeping 6x, the timer should have fired at 1, 3, 5
			test::Sleep(6 * timeUnit);
			if (!EXPECT_EQ_RETRY(3u, counter))
				return false;

			EXPECT_EQ(1u, context.counter(Counter_Name));
			return true;
		});
	}

	// endregion
}}

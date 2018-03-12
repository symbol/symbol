#include "SchedulerService.h"
#include "TasksConfiguration.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/thread/Scheduler.h"

namespace catapult { namespace sync {

	namespace {
		constexpr auto Service_Name = "scheduler";

		class SchedulerServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit SchedulerServiceRegistrar(const TasksConfiguration& config) : m_config(config)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Scheduler", extensions::ServiceRegistrarPhase::Post_Tasks };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<thread::Scheduler>(Service_Name, "TASKS", [](const auto& scheduler) {
					return scheduler.numScheduledTasks();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				auto pScheduler = state.pool().pushServiceGroup(Service_Name)->pushService(thread::CreateScheduler);
				for (const auto& task : state.tasks())
					pScheduler->addTask(schedule(task));

				locator.registerService(Service_Name, pScheduler);
			}

		private:
			thread::Task schedule(const thread::Task& unscheduledTask) const {
				auto iter = m_config.Tasks.find(unscheduledTask.Name);
				if (m_config.Tasks.cend() == iter)
					CATAPULT_THROW_INVALID_ARGUMENT_1("unable to schedule task without config entry", unscheduledTask.Name);

				auto scheduledTask = unscheduledTask;
				scheduledTask.StartDelay = iter->second.StartDelay;
				scheduledTask.RepeatDelay = iter->second.RepeatDelay;
				return scheduledTask;
			}

		private:
			TasksConfiguration m_config;
		};
	}

	DECLARE_SERVICE_REGISTRAR(Scheduler)(const TasksConfiguration& config) {
		return std::make_unique<SchedulerServiceRegistrar>(config);
	}
}}

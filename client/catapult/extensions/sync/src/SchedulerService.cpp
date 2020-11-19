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
				if (TasksConfiguration::TaskType::Uniform == iter->second.TaskType) {
					const auto& taskConfig = iter->second.Uniform;
					scheduledTask.StartDelay = taskConfig.StartDelay;
					scheduledTask.NextDelay = thread::CreateUniformDelayGenerator(taskConfig.RepeatDelay);
				} else {
					const auto& taskConfig = iter->second.Decelerating;
					scheduledTask.StartDelay = taskConfig.StartDelay;
					scheduledTask.NextDelay = thread::CreateIncreasingDelayGenerator(
							taskConfig.MinDelay,
							taskConfig.NumPhaseOneRounds,
							taskConfig.MaxDelay,
							taskConfig.NumTransitionRounds);
				}

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

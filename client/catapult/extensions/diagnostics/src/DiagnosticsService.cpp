#include "DiagnosticsService.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/handlers/DiagnosticHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace diagnostics {

	namespace {
		thread::Task CreateLoggingTask(const std::vector<utils::DiagnosticCounter>& counters) {
			return thread::CreateNamedTask("logging task", [counters]() {
				std::ostringstream table;
				table << "--- current counter values ---";
				for (const auto& counter : counters) {
					table.width(utils::DiagnosticCounterId::Max_Counter_Name_Size);
					table << std::endl << counter.id().name() << " : " << counter.value();
				}

				CATAPULT_LOG(info) << table.str();
				return thread::make_ready_future(thread::TaskResult::Continue);
			});
		}

		void AddDiagnosticHandlers(const std::vector<utils::DiagnosticCounter>& counters, extensions::ServiceState& state) {
			auto& handlers = state.packetHandlers();
			handlers::RegisterDiagnosticCountersHandler(handlers, counters);
			handlers::RegisterDiagnosticNodesHandler(handlers, state.nodes());
			state.pluginManager().addDiagnosticHandlers(handlers, state.cache());
		}

		class DiagnosticsServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Diagnostics", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// merge all counters
				auto counters = state.counters();
				counters.insert(counters.end(), locator.counters().cbegin(), locator.counters().cend());

				// add task
				state.tasks().push_back(CreateLoggingTask(counters));

				// add packet handlers
				AddDiagnosticHandlers(counters, state);
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(Diagnostics)() {
		return std::make_unique<DiagnosticsServiceRegistrar>();
	}
}}

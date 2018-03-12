#include "RootedService.h"
#include "ServiceLocator.h"

namespace catapult { namespace extensions {

	namespace {
		class RootedServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			RootedServiceRegistrar(const std::shared_ptr<void>& pService, const std::string& serviceName, ServiceRegistrarPhase phase)
					: m_pService(pService)
					, m_serviceName(serviceName)
					, m_phase(phase)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Rooted - " + m_serviceName, m_phase };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState&) override {
				// register rooted service
				locator.registerRootedService(m_serviceName, m_pService);
			}

		private:
			std::shared_ptr<void> m_pService;
			std::string m_serviceName;
			ServiceRegistrarPhase m_phase;
		};
	}

	DECLARE_SERVICE_REGISTRAR(Rooted)(const std::shared_ptr<void>& pService, const std::string& serviceName, ServiceRegistrarPhase phase) {
		return std::make_unique<RootedServiceRegistrar>(pService, serviceName, phase);
	}
}}

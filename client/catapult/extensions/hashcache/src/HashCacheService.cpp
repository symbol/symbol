#include "HashCacheService.h"
#include "plugins/services/hashcache/src/cache/HashCachePredicates.h"
#include "catapult/extensions/ServiceState.h"

namespace catapult { namespace hashcache {

	namespace {
		class HashCacheServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "HashCache", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator&, extensions::ServiceState& state) override {
				state.hooks().addKnownHashPredicate([&cache = state.cache()](auto timestamp, const auto& hash) {
					return cache::HashCacheContains(cache, timestamp, hash);
				});
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(HashCache)() {
		return std::make_unique<HashCacheServiceRegistrar>();
	}
}}

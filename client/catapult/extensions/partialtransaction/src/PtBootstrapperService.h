#pragma once
#include "PtTypes.h"
#include "catapult/extensions/BasicServerHooks.h"
#include "catapult/extensions/ServiceRegistrar.h"
#include "catapult/handlers/HandlerTypes.h"

namespace catapult { namespace cache { class MemoryPtCacheProxy; } }

namespace catapult { namespace partialtransaction {

	// region PtServerHooks

	/// Hooks for the partial transactions subsystem.
	class PtServerHooks {
	public:
		/// Sets the cosigned transaction infos \a consumer.
		void setCosignedTransactionInfosConsumer(const CosignedTransactionInfosConsumer& consumer) {
			extensions::SetOnce(m_cosignedTransactionInfosConsumer, consumer);
		}

		/// Sets the partial transaction range \a consumer.
		void setPtRangeConsumer(const handlers::TransactionRangeHandler& consumer) {
			extensions::SetOnce(m_ptRangeConsumer, consumer);
		}

		/// Sets the cosignature range \a consumer.
		void setCosignatureRangeConsumer(const handlers::RangeHandler<model::DetachedCosignature>& consumer) {
			extensions::SetOnce(m_cosignatureRangeConsumer, consumer);
		}

	public:
		/// Gets the cosigned transaction infos consumer.
		const auto& cosignedTransactionInfosConsumer() const {
			return extensions::Require(m_cosignedTransactionInfosConsumer);
		}

		/// Gets the partial transaction range consumer.
		const auto& ptRangeConsumer() const {
			return extensions::Require(m_ptRangeConsumer);
		}

		/// Gets the cosignature range consumer.
		const auto& cosignatureRangeConsumer() const {
			return extensions::Require(m_cosignatureRangeConsumer);
		}

	private:
		CosignedTransactionInfosConsumer m_cosignedTransactionInfosConsumer;
		handlers::TransactionRangeHandler m_ptRangeConsumer;
		handlers::RangeHandler<model::DetachedCosignature> m_cosignatureRangeConsumer;
	};

	// endregion

	/// Partial transactions cache supplier.
	using PtCacheSupplier = supplier<std::unique_ptr<cache::MemoryPtCacheProxy>>;

	/// Creates a registrar for a partial transactions bootstrapper service given the cache supplier \a ptCacheSupplier.
	/// \note This service is responsible for registering root partial transactions services.
	/// \note Cache supplier needs to be passed instead of cache in order to allow all other extensions to register subscriptions first
	///       since subscriptions are bound when cache is created.
	DECLARE_SERVICE_REGISTRAR(PtBootstrapper)(const PtCacheSupplier& ptCacheSupplier);

	/// Gets the memory partial transactions cache stored in \a locator.
	cache::MemoryPtCacheProxy& GetMemoryPtCache(const extensions::ServiceLocator& locator);

	/// Gets the partial transactions server hooks stored in \a locator.
	PtServerHooks& GetPtServerHooks(const extensions::ServiceLocator& locator);
}}

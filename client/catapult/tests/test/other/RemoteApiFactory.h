#pragma once
#include "catapult/ionet/PacketIo.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace test {

	/// Creates a remote api around \a io using \a apiFactory such that the returned api extends the lifetime of \a registry.
	template<typename TRemoteApiFactory>
	auto CreateLifetimeExtendedApi(TRemoteApiFactory apiFactory, ionet::PacketIo& io, model::TransactionRegistry&& registry) {
		auto pRegistry = std::make_shared<model::TransactionRegistry>(std::move(registry));
		auto pRemoteApi = utils::UniqueToShared(apiFactory(io, *pRegistry));
		return decltype(pRemoteApi)(pRemoteApi.get(), [pRegistry, pRemoteApi](const auto*) {});
	}

	/// Creates a remote api around \a pIo using \a apiFactory such that the returned api extends the lifetime of both
	/// \a pIo and \a registry.
	template<typename TRemoteApiFactory>
	auto CreateLifetimeExtendedApi(
			TRemoteApiFactory apiFactory,
			const std::shared_ptr<ionet::PacketIo>& pIo,
			model::TransactionRegistry&& registry) {
		auto pRegistry = std::make_shared<model::TransactionRegistry>(std::move(registry));
		auto pRemoteApi = utils::UniqueToShared(apiFactory(*pIo, *pRegistry));
		return decltype(pRemoteApi)(pRemoteApi.get(), [pIo, pRegistry, pRemoteApi](const auto*) {});
	}
}}

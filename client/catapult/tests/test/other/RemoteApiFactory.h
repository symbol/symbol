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

#pragma once
#include "catapult/ionet/PacketIo.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace test {

	/// Creates a remote api around \a io with specified \a remoteIdentity using \a apiFactory such that the returned api
	/// extends the lifetime of \a registry.
	template<typename TRemoteApiFactory>
	auto CreateLifetimeExtendedApi(
			TRemoteApiFactory apiFactory,
			ionet::PacketIo& io,
			const model::NodeIdentity& remoteIdentity,
			model::TransactionRegistry&& registry) {
		auto pRegistry = std::make_shared<model::TransactionRegistry>(std::move(registry));
		auto pRemoteApi = utils::UniqueToShared(apiFactory(io, remoteIdentity, *pRegistry));
		return decltype(pRemoteApi)(pRemoteApi.get(), [pRegistry, pRemoteApi](const auto*) {});
	}

	/// Creates a remote api around \a pIo with specified \a remoteIdentity using \a apiFactory such that the returned api
	/// extends the lifetime of both \a pIo and \a registry.
	template<typename TRemoteApiFactory>
	auto CreateLifetimeExtendedApi(
			TRemoteApiFactory apiFactory,
			const std::shared_ptr<ionet::PacketIo>& pIo,
			const model::NodeIdentity& remoteIdentity,
			model::TransactionRegistry&& registry) {
		auto pRegistry = std::make_shared<model::TransactionRegistry>(std::move(registry));
		auto pRemoteApi = utils::UniqueToShared(apiFactory(*pIo, remoteIdentity, *pRegistry));
		return decltype(pRemoteApi)(pRemoteApi.get(), [pIo, pRegistry, pRemoteApi](const auto*) {});
	}
}}

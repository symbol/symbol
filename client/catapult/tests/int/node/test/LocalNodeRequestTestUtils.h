/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "ExternalSourceConnection.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/model/BlockUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include "tests/test/nodeps/Waits.h"

namespace catapult { namespace test {

	// region boot

	/// Waits for \a context to be booted.
	template<typename TTestContext>
	void WaitForBoot(const TTestContext& context) {
		WAIT_FOR_ONE_EXPR(context.stats().NumActiveWriters);
	}

	// endregion

	// region push

	/// Pushes \a payload to \a connection.
	std::shared_ptr<ionet::PacketIo> PushPayload(ExternalSourceConnection& connection, const ionet::PacketPayload& payload);

	/// Pushes \a pEntity as a packet of \a type to \a connection.
	template<typename TEntity>
	std::shared_ptr<ionet::PacketIo> PushEntity(
			ExternalSourceConnection& connection,
			ionet::PacketType type,
			const std::shared_ptr<TEntity>& pEntity) {
		return PushPayload(connection, ionet::PacketPayloadFactory::FromEntity(type, pEntity));
	}

	/// Pushes \a entities as a packet of \a type to \a connection.
	template<typename TEntity>
	std::shared_ptr<ionet::PacketIo> PushEntities(
			ExternalSourceConnection& connection,
			ionet::PacketType type,
			const std::vector<std::shared_ptr<TEntity>>& entities) {
		return PushPayload(connection, ionet::PacketPayloadFactory::FromEntities(type, entities));
	}

	/// Pushes a valid block to \a connection.
	std::shared_ptr<ionet::PacketIo> PushValidBlock(ExternalSourceConnection& connection);

	/// Pushes a valid transaction to \a connection.
	std::shared_ptr<ionet::PacketIo> PushValidTransaction(ExternalSourceConnection& connection);

	// endregion

	// region pull

	/// Asserts that a response can be retrieved for the specified traits using \a context and then calls \a handler.
	template<typename TApiTraits, typename TTestContext, typename THandler>
	void AssertCanGetResponse(TTestContext&& context, THandler handler) {
		// Arrange:
		WaitForBoot(context);

		// Act:
		std::atomic_bool isReadFinished(false);
		bool isExceptionRaised = false;
		typename TApiTraits::ResultType result;
		ExternalSourceConnection connection(context.publicKey());
		connection.apiCall([&](const auto& pRemoteChainApi) {
			TApiTraits::InitiateValidRequest(*pRemoteChainApi).then([&, pRemoteChainApi](auto&& future) {
				try {
					result = std::move(future.get());
				} catch (const catapult_runtime_error& ex) {
					CATAPULT_LOG(debug) << "exception thrown by get " << ex.what();
					isExceptionRaised = true;
				}

				isReadFinished = true;
			});
		});

		WAIT_FOR(isReadFinished);

		// Assert:
		EXPECT_FALSE(isExceptionRaised);
		TApiTraits::VerifyResult(result);
		handler(context.stats());
	}

	/// Asserts that the request initiated by \a requestInitator triggers an api exception using \a context
	/// and then calls \a handler.
	template<typename TTestContext, typename TRequestInitiator, typename THandler>
	void AssertInvalidRequestTriggersException(TTestContext&& context, TRequestInitiator requestInitator, THandler handler) {
		// Arrange:
		WaitForBoot(context);

		// Act:
		std::atomic_bool isReadFinished(false);
		ExternalSourceConnection connection(context.publicKey());
		connection.apiCall([&](const auto& pRemoteChainApi) {
			requestInitator(*pRemoteChainApi).then([&, pRemoteChainApi](auto&& future) {
				// Act + Assert:
				EXPECT_THROW(future.get(), catapult_runtime_error);
				isReadFinished = true;
			});
		});

		WAIT_FOR(isReadFinished);

		// Assert:
		handler(context.stats());
	}

	// endregion

	// region height

	/// Gets the node height via \a connection.
	Height GetLocalNodeHeightViaApi(ExternalSourceConnection& connection);

	/// Waits for node height reported to \a connection to equal \a height.
	void WaitForLocalNodeHeight(ExternalSourceConnection& connection, Height height);

	// endregion
}}

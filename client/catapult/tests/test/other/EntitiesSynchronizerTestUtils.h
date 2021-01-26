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
#include "catapult/model/AnnotatedEntityRange.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Tests for an entities synchronizer.
	template<typename TTraits>
	class EntitiesSynchronizerTests {
	private:
		using RequestRangeType = model::EntityRange<typename TTraits::RequestElementType>;
		using ResponseContainerType = typename TTraits::ResponseContainerType;

	private:
		struct TestContext {
			explicit TestContext(const RequestRangeType& localRequestRange)
					: pIo(std::make_shared<mocks::MockPacketIo>())
					, RequestRangeSupplierCalls(0)
					, RequestRange(RequestRangeType::CopyRange(localRequestRange))
					, ResponseContainerConsumerCalls(0)
			{}

			std::shared_ptr<mocks::MockPacketIo> pIo;

			// request sent to the remote
			size_t RequestRangeSupplierCalls;
			RequestRangeType RequestRange;

			// response received from the remote
			size_t ResponseContainerConsumerCalls;
			ResponseContainerType ConsumerResponseContainer;
		};

	private:
		template<typename ...TArgs>
		static auto CreateSynchronizer(TestContext& context, TArgs... args) {
			auto& requestRangeSupplierCalls = context.RequestRangeSupplierCalls;
			const auto& requestRange = context.RequestRange;
			auto requestRangeSupplier = [&requestRangeSupplierCalls, &requestRange]() {
				++requestRangeSupplierCalls;
				return RequestRangeType::CopyRange(requestRange);
			};

			auto& responseContainerConsumerCalls = context.ResponseContainerConsumerCalls;
			auto& capturedResponseContainer = context.ConsumerResponseContainer;
			auto responseContainerConsumer = [&responseContainerConsumerCalls, &capturedResponseContainer](auto&& responseContainer) {
				++responseContainerConsumerCalls;
				capturedResponseContainer = std::move(responseContainer);
			};

			return TTraits::CreateSynchronizer(requestRangeSupplier, responseContainerConsumer, std::forward<TArgs>(args)...);
		}

		template<typename TResponseContainer, typename TResponseEntity>
		static void AssertResponse(
				const TResponseContainer& expectedResponse,
				const model::NodeIdentity& expectedSourceIdentity,
				const model::AnnotatedEntityRange<TResponseEntity>& actualResponse) {
			// Assert: range contains expected contents and has expected source public key
			test::AssertEqualRange(expectedResponse, actualResponse.Range, "response");
			EXPECT_EQ(expectedSourceIdentity.PublicKey, actualResponse.SourceIdentity.PublicKey);
			EXPECT_EQ(expectedSourceIdentity.Host, actualResponse.SourceIdentity.Host);
		}

		template<typename TResponseContainer>
		static void AssertResponse(
				const TResponseContainer& expectedResponse,
				const model::NodeIdentity&,
				const TResponseContainer& actualResponse) {
			// Assert: response contains expected contents
			TTraits::AssertCustomResponse(expectedResponse, actualResponse);
		}

	public:
		/// Asserts a successful interaction when new data is pulled.
		static void AssertSuccessInteractionWhenNewDataIsPulled() {
			// Arrange:
			// - create the synchronizer
			auto requestRange = TTraits::CreateRequestRange(5);
			TestContext context(requestRange);
			auto synchronizer = CreateSynchronizer(context);

			// - create the api
			auto responseContainer = TTraits::CreateResponseContainer(5);
			auto remoteApiWrapper = TTraits::CreateRemoteApi(responseContainer);

			// Act:
			auto code = synchronizer(remoteApiWrapper.api()).get();

			// Assert: check result and counters
			EXPECT_EQ(ionet::NodeInteractionResultCode::Success, code);
			EXPECT_EQ(1u, context.RequestRangeSupplierCalls);
			EXPECT_EQ(1u, context.ResponseContainerConsumerCalls);

			// - check request range
			ASSERT_EQ(1u, remoteApiWrapper.numCalls());
			test::AssertEqualRange(requestRange, remoteApiWrapper.singleRequest(), "request");
			remoteApiWrapper.checkAdditionalRequestParameters();

			// - check response container
			AssertResponse(responseContainer, remoteApiWrapper.api().remoteIdentity(), context.ConsumerResponseContainer);
		}

		/// Asserts a neutral interaction when no data is pulled.
		static void AssertNeutralInteractionWhenNoDataIsPulled() {
			// Arrange:
			// - create the synchronizer
			auto requestRange = TTraits::CreateRequestRange(5);
			TestContext context(requestRange);
			auto synchronizer = CreateSynchronizer(context);

			// - create the api
			auto remoteApiWrapper = TTraits::CreateRemoteApi({});

			// Act:
			auto code = synchronizer(remoteApiWrapper.api()).get();

			// Assert: check result and counters
			EXPECT_EQ(ionet::NodeInteractionResultCode::Neutral, code);
			EXPECT_EQ(1u, context.RequestRangeSupplierCalls);
			EXPECT_EQ(0u, context.ResponseContainerConsumerCalls);

			// - check request range
			ASSERT_EQ(1u, remoteApiWrapper.numCalls());
			test::AssertEqualRange(requestRange, remoteApiWrapper.singleRequest(), "request");
			remoteApiWrapper.checkAdditionalRequestParameters();

			// - no response container
		}

		/// Asserts a failed interaction when remote api throws.
		static void AssertFailedInteractionWhenRemoteApiThrows() {
			// Arrange:
			// - create the synchronizer
			auto requestRange = TTraits::CreateRequestRange(5);
			TestContext context(requestRange);
			auto synchronizer = CreateSynchronizer(context);

			// - create the api
			auto responseContainer = TTraits::CreateResponseContainer(5);
			auto remoteApiWrapper = TTraits::CreateRemoteApi(responseContainer);
			remoteApiWrapper.setError();

			// Act:
			auto code = synchronizer(remoteApiWrapper.api()).get();

			// Assert: check result and counters
			EXPECT_EQ(ionet::NodeInteractionResultCode::Failure, code);
			EXPECT_EQ(1u, context.RequestRangeSupplierCalls);
			EXPECT_EQ(0u, context.ResponseContainerConsumerCalls);

			// - check request range
			ASSERT_EQ(1u, remoteApiWrapper.numCalls());
			test::AssertEqualRange(requestRange, remoteApiWrapper.singleRequest(), "request");
			remoteApiWrapper.checkAdditionalRequestParameters();

			// - no response container
		}

		/// Asserts that a successful interaction can follow a failed interaction.
		static void AssertCanRecoverAfterFailedInteraction() {
			// Arrange:
			// - create the synchronizer
			auto requestRange = TTraits::CreateRequestRange(5);
			TestContext context(requestRange);
			auto synchronizer = CreateSynchronizer(context);

			// - create the api
			auto responseContainer = TTraits::CreateResponseContainer(5);
			auto remoteApiWrapper = TTraits::CreateRemoteApi(responseContainer);

			std::vector<ionet::NodeInteractionResultCode> interactionResultCodes;

			// Act: set an exception and sync
			remoteApiWrapper.setError();
			interactionResultCodes.push_back(synchronizer(remoteApiWrapper.api()).get());

			// - clear the exception and sync
			remoteApiWrapper.setError(false);
			interactionResultCodes.push_back(synchronizer(remoteApiWrapper.api()).get());

			// Assert: the first sync failed but the second succeeded
			EXPECT_EQ(2u, context.RequestRangeSupplierCalls);
			EXPECT_EQ(1u, context.ResponseContainerConsumerCalls);

			std::vector<ionet::NodeInteractionResultCode> expectedInteractionResultCodes{
				ionet::NodeInteractionResultCode::Failure,
				ionet::NodeInteractionResultCode::Success
			};
			EXPECT_EQ(expectedInteractionResultCodes, interactionResultCodes);
		}

		/// Asserts a neutral interaction when conditional predicate returns \c false.
		static void AssertNeutralInteractionWhenConditionBypassesRequest() {
			// Arrange:
			// - create the synchronizer
			auto requestRange = TTraits::CreateRequestRange(5);
			TestContext context(requestRange);
			auto synchronizer = CreateSynchronizer(context, false);

			// - create the api
			auto responseContainer = TTraits::CreateResponseContainer(5);
			auto remoteApiWrapper = TTraits::CreateRemoteApi(responseContainer);

			// Act:
			auto code = synchronizer(remoteApiWrapper.api()).get();

			// Assert: check result and counters
			EXPECT_EQ(ionet::NodeInteractionResultCode::Neutral, code);
			EXPECT_EQ(0u, context.RequestRangeSupplierCalls);
			EXPECT_EQ(0u, context.ResponseContainerConsumerCalls);

			// - check request range
			EXPECT_EQ(0u, remoteApiWrapper.numCalls());
		}
	};

#define MAKE_ENTITIES_SYNCHRONIZER_TEST(TEST_CLASS, TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::EntitiesSynchronizerTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_ENTITIES_SYNCHRONIZER_TESTS(SYNCHRONIZER_NAME) \
	MAKE_ENTITIES_SYNCHRONIZER_TEST(SYNCHRONIZER_NAME##Tests, SYNCHRONIZER_NAME##Traits, SuccessInteractionWhenNewDataIsPulled) \
	MAKE_ENTITIES_SYNCHRONIZER_TEST(SYNCHRONIZER_NAME##Tests, SYNCHRONIZER_NAME##Traits, NeutralInteractionWhenNoDataIsPulled) \
	MAKE_ENTITIES_SYNCHRONIZER_TEST(SYNCHRONIZER_NAME##Tests, SYNCHRONIZER_NAME##Traits, FailedInteractionWhenRemoteApiThrows) \
	MAKE_ENTITIES_SYNCHRONIZER_TEST(SYNCHRONIZER_NAME##Tests, SYNCHRONIZER_NAME##Traits, CanRecoverAfterFailedInteraction)

#define DEFINE_CONDITIONAL_ENTITIES_SYNCHRONIZER_TESTS(SYNCHRONIZER_NAME) \
	DEFINE_ENTITIES_SYNCHRONIZER_TESTS(SYNCHRONIZER_NAME) \
	MAKE_ENTITIES_SYNCHRONIZER_TEST(SYNCHRONIZER_NAME##Tests, SYNCHRONIZER_NAME##Traits, NeutralInteractionWhenConditionBypassesRequest)
}}

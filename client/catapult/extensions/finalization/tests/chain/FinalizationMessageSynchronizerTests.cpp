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

#include "finalization/src/chain/FinalizationMessageSynchronizer.h"
#include "finalization/tests/test/mocks/MockFinalizationApi.h"
#include "tests/test/other/EntitiesSynchronizerTestUtils.h"
#include "tests/TestHarness.h"

#define DEFINE_MODEL_BINARY_EQUALITY(NAMESPACE, TYPENAME) \
	namespace catapult { namespace NAMESPACE { \
		bool operator==(const TYPENAME& lhs, const TYPENAME& rhs); \
		bool operator==(const TYPENAME& lhs, const TYPENAME& rhs) { \
			return lhs.Size == rhs.Size && 0 == std::memcmp(&lhs, &rhs, lhs.Size); \
		} \
	}}

DEFINE_MODEL_BINARY_EQUALITY(model, FinalizationMessage)

namespace catapult { namespace chain {

	namespace {
		using MockRemoteApi = mocks::MockFinalizationApi;
		using ShortHashesSupplier = supplier<model::ShortHashRange>;

		class FinalizationMessageSynchronizerTraits {
		public:
			using RequestElementType = utils::ShortHash;
			using ResponseContainerType = model::AnnotatedEntityRange<model::FinalizationMessage>;

		public:
			class RemoteApiWrapper {
			public:
				explicit RemoteApiWrapper(const model::FinalizationMessageRange& messageRange)
						: m_pFinalizationApi(std::make_unique<MockRemoteApi>(messageRange))
				{}

			public:
				const auto& api() const {
					return *m_pFinalizationApi;
				}

				auto numCalls() const {
					return m_pFinalizationApi->messagesRequests().size();
				}

				const auto& singleRequest() const {
					return m_pFinalizationApi->messagesRequests()[0].second;
				}

				void setError(bool setError = true) {
					auto entryPoint = setError
							? MockRemoteApi::EntryPoint::Messages
							: MockRemoteApi::EntryPoint::None;
					m_pFinalizationApi->setError(entryPoint);
				}

				void checkAdditionalRequestParameters() {
					EXPECT_EQ(FinalizationPoint(23), m_pFinalizationApi->messagesRequests()[0].first);
				}

			private:
				std::unique_ptr<MockRemoteApi> m_pFinalizationApi;
			};

		public:
			static auto CreateRequestRange(uint32_t count) {
				auto shortHashes = test::GenerateRandomDataVector<utils::ShortHash>(count);
				return model::ShortHashRange::CopyFixed(reinterpret_cast<const uint8_t*>(shortHashes.data()), count);
			}

			static auto CreateResponseContainer(uint32_t count) {
				constexpr auto Message_Size = sizeof(model::FinalizationMessage);

				auto buffer = test::GenerateRandomVector(count * Message_Size);
				for (auto i = 0u; i < count; ++i) {
					auto& message = reinterpret_cast<model::FinalizationMessage&>(buffer[i * Message_Size]);
					message.Size = Message_Size;
					message.HashesCount = 0;
				}

				return model::FinalizationMessageRange::CopyFixed(buffer.data(), count);
			}

			static auto CreateRemoteApi(const model::FinalizationMessageRange& messageRange) {
				return RemoteApiWrapper(messageRange);
			}

			static auto CreateSynchronizer(
					const ShortHashesSupplier& shortHashesSupplier,
					const handlers::MessageRangeHandler& messageRangeConsumer) {
				auto messageFilterSupplier = [shortHashesSupplier] {
					return std::make_pair(FinalizationPoint(23), shortHashesSupplier());
				};
				return CreateFinalizationMessageSynchronizer(messageFilterSupplier, messageRangeConsumer);
			}
		};
	}

	DEFINE_ENTITIES_SYNCHRONIZER_TESTS(FinalizationMessageSynchronizer)
}}

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
#include "FinalizationMessageTestUtils.h"
#include "finalization/src/FinalizationBootstrapperService.h"
#include "catapult/cache/CatapultCache.h"
#include "tests/test/local/ServiceLocatorTestContext.h"

namespace catapult { namespace test {

	// region FinalizationBootstrapperServiceTestUtils

	/// Helpers for initializing the bootstrapper service.
	class FinalizationBootstrapperServiceTestUtils {
	public:
		/// Number of expected bootstrapper services.
		static constexpr auto Num_Bootstrapper_Services = 4u;

		/// Types of accounts registered by CreateCache.
		enum class VoterType : uint32_t { Small, Large1, Ineligible, Large2 };

	public:
		/// Creates an empty catapult cache with all required sub caches.
		static cache::CatapultCache CreateCache();

		/// Creates a catapult cache with all required sub caches and seed accounts.
		/// \note \a keyPairDescriptors is updated with account information.
		static cache::CatapultCache CreateCache(std::vector<AccountKeyPairDescriptor>& keyPairDescriptors);

	public:
		/// Registers the bootstrapper service with \a locator given \a state.
		static void Register(extensions::ServiceLocator& locator, extensions::ServiceState& state);

		/// Registers the bootstrapper service with \a locator given \a state and \a pProofStorage.
		static void Register(
				extensions::ServiceLocator& locator,
				extensions::ServiceState& state,
				std::unique_ptr<io::ProofStorage>&& pProofStorage);
	};

	// endregion

	// region MessageRangeConsumerDependentServiceLocatorTestContext

	/// Test context to use for a service dependent on the bootstrap service and a valid message range consumer.
	template<typename TTraits>
	class MessageRangeConsumerDependentServiceLocatorTestContext : public ServiceLocatorTestContext<TTraits> {
	private:
		using BaseTestContext = ServiceLocatorTestContext<TTraits>;
		using TestUtils = FinalizationBootstrapperServiceTestUtils;

	public:
		MessageRangeConsumerDependentServiceLocatorTestContext() : BaseTestContext(TestUtils::CreateCache()) {
			// Arrange: register service dependencies
			TestUtils::Register(BaseTestContext::locator(), BaseTestContext::testState().state());

			// - register hook dependencies
			auto& hooks = finalization::GetFinalizationServerHooks(BaseTestContext::locator());
			hooks.setMessageRangeConsumer([](auto&&) {});
		}
	};

	// endregion

	// region VoterSeededCacheDependentServiceLocatorTestContext

	/// Test context to use for a service dependent on the bootstrap service and a cache seeded with representative accounts.
	template<typename TTraits>
	class VoterSeededCacheDependentServiceLocatorTestContext : public ServiceLocatorTestContext<TTraits> {
	private:
		using KeyPairDescriptors = std::vector<AccountKeyPairDescriptor>;
		using TestUtils = FinalizationBootstrapperServiceTestUtils;

	public:
		/// Creates the test context.
		VoterSeededCacheDependentServiceLocatorTestContext()
				: VoterSeededCacheDependentServiceLocatorTestContext(std::make_unique<KeyPairDescriptors>())
		{}

	private:
		VoterSeededCacheDependentServiceLocatorTestContext(std::unique_ptr<KeyPairDescriptors>&& pKeyPairDescriptors)
				: ServiceLocatorTestContext<TTraits>(TestUtils::CreateCache(*pKeyPairDescriptors))
				, m_keyPairDescriptors(std::move(*pKeyPairDescriptors))
		{}

	protected:
		/// Gets the key pair descriptor for the account specified by \a voterType.
		const AccountKeyPairDescriptor& keyPairDescriptor(TestUtils::VoterType voterType) const {
			return m_keyPairDescriptors[utils::to_underlying_type(voterType)];
		}

	public:
		/// Creates a valid finalization message with \a stepIdentifier and one \a hash at \a height for the account
		/// specified by \a voterType.
		std::shared_ptr<model::FinalizationMessage> createMessage(
				TestUtils::VoterType voterType,
				const model::StepIdentifier& stepIdentifier,
				Height height,
				const Hash256& hash) const {
			auto pMessage = CreateMessage(stepIdentifier, hash);
			pMessage->Data().Height = height;
			SignMessage(*pMessage, keyPairDescriptor(voterType).VotingKeyPair);
			return PORTABLE_MOVE(pMessage);
		}

	private:
		KeyPairDescriptors m_keyPairDescriptors;
	};

	// endregion
}}

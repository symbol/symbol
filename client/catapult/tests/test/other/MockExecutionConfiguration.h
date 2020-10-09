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
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/BlockStatisticCache.h"
#include "catapult/chain/ExecutionConfiguration.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/utils/Hashers.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"
#include <unordered_map>

namespace catapult { namespace test {

	constexpr auto Mock_Execution_Configuration_Network_Identifier = model::NetworkIdentifier::Private_Test;

	// region MockNotificationPublisher

	struct PublisherParams {
	public:
		explicit PublisherParams(const model::WeakEntityInfo& entityInfo)
				: EntityInfo(entityInfo)
				, HashCopy(entityInfo.hash())
		{}

	public:
		const model::WeakEntityInfo EntityInfo;
		const Hash256 HashCopy;
	};

	struct MockNotification : public model::Notification {
	public:
		static constexpr auto Notification_Type = static_cast<model::NotificationType>(std::numeric_limits<uint32_t>::max());

	public:
		MockNotification(const Hash256& hash, uint64_t id)
				: Notification(Notification_Type, sizeof(MockNotification))
				, Hash(hash)
				, Id(id)
		{}

	public:
		const Hash256 Hash;
		uint64_t Id;
	};

	class MockNotificationPublisher : public model::NotificationPublisher, public ParamsCapture<PublisherParams> {
	public:
		MockNotificationPublisher() : m_emulatePublicKeyNotifications(false)
		{}

	public:
		void publish(const model::WeakEntityInfo& entityInfo, model::NotificationSubscriber& subscriber) const override {
			const_cast<MockNotificationPublisher*>(this)->push(entityInfo);
			subscriber.notify(MockNotification(entityInfo.hash(), 1));
			subscriber.notify(MockNotification(entityInfo.hash(), 2));

			if (m_emulatePublicKeyNotifications)
				subscriber.notify(model::AccountPublicKeyNotification(reinterpret_cast<const Key&>(entityInfo.hash())));
		}

	public:
		/// Emulates account public key notifications by raising notification with hash coerced to public key.
		/// \note Tests that rely on this behavior are not using MockTransaction.
		void emulatePublicKeyNotifications() {
			m_emulatePublicKeyNotifications = true;
		}

	private:
		bool m_emulatePublicKeyNotifications;
	};

	// endregion

	// region MockAggregateNotificationObserver

	struct ObserverParams {
	public:
		ObserverParams(const MockNotification& notification, const observers::ObserverContext& context)
				: HashCopy(notification.Hash)
				, SequenceId(notification.Id)
				, Context(context)
				, IsPassedMarkedCache(IsMarkedCache(context.Cache))
				, NumStatistics(context.Cache.sub<cache::BlockStatisticCache>().size())
				, StateCopy(context.Cache.dependentState()) // make a copy of the state
		{}

	public:
		const Hash256 HashCopy;
		const size_t SequenceId;
		const observers::ObserverContext Context;
		const bool IsPassedMarkedCache;
		const size_t NumStatistics;
		const state::CatapultState StateCopy;
	};

	class MockAggregateNotificationObserver : public observers::AggregateNotificationObserver, public ParamsCapture<ObserverParams> {
	public:
		MockAggregateNotificationObserver()
				: m_name("MockAggregateNotificationObserver")
				, m_enableRollbackEmulation(false)
				, m_enableReceiptGeneration(false)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		std::vector<std::string> names() const override {
			return { name() };
		}

		void notify(const model::Notification& notification, observers::ObserverContext& context) const override {
			auto& accountStateCacheDelta = context.Cache.sub<cache::AccountStateCache>();
			if (model::Core_Register_Account_Public_Key_Notification == notification.Type) {
				// simulate AccountPublicKeyObserver
				const auto& publicKey = CastToDerivedNotification<model::AccountPublicKeyNotification>(notification).PublicKey;
				if (observers::NotifyMode::Commit == context.Mode)
					accountStateCacheDelta.addAccount(publicKey, context.Height);
				else
					accountStateCacheDelta.queueRemove(publicKey, context.Height);
			} else if (model::Core_Register_Account_Address_Notification == notification.Type) {
				// simulate AccountAddressObserver
				const auto& addressNotification = CastToDerivedNotification<model::AccountAddressNotification>(notification);
				auto address = addressNotification.Address.resolved(context.Resolvers);
				if (observers::NotifyMode::Commit == context.Mode)
					accountStateCacheDelta.addAccount(address, context.Height);
				else
					accountStateCacheDelta.queueRemove(address, context.Height);
			} else if (MockNotification::Notification_Type == notification.Type) {
				const auto& mockNotification = CastToDerivedNotification<MockNotification>(notification);
				const_cast<MockAggregateNotificationObserver*>(this)->push(mockNotification, context);

				addBlockStatisticBreadcrumb(context);
				addReceiptBreadcrumb(context, mockNotification);
			} else {
				CATAPULT_THROW_RUNTIME_ERROR("MockAggregateNotificationObserver received unexpected notification");
			}
		}

	private:
		void addBlockStatisticBreadcrumb(observers::ObserverContext& context) const {
			auto& blockStatisticCache = context.Cache.sub<cache::BlockStatisticCache>();
			auto markerId = blockStatisticCache.size();

			if (!m_enableRollbackEmulation || observers::NotifyMode::Commit == context.Mode)
				blockStatisticCache.insert(state::BlockStatistic(Height(markerId + 1)));
			else
				blockStatisticCache.remove(state::BlockStatistic(Height(markerId)));
		}

		void addReceiptBreadcrumb(observers::ObserverContext& context, const MockNotification& mockNotification) const {
			if (!m_enableReceiptGeneration || observers::NotifyMode::Commit != context.Mode)
				return;

			auto& blockStatisticCache = context.Cache.sub<cache::BlockStatisticCache>();
			AddReceiptBreadcrumb(context.StatementBuilder(), mockNotification.Id, blockStatisticCache.size());
		}

	public:
		/// Enables rollback emulation.
		void enableRollbackEmulation() {
			m_enableRollbackEmulation = true;
		}

		/// Enables generation of a receipt for every notify (commit) call.
		void enableReceiptGeneration() {
			m_enableReceiptGeneration = true;
		}

	private:
		static void AddReceiptBreadcrumb(observers::ObserverStatementBuilder& statementBuilder, size_t sequenceId, size_t breadcrumbId) {
			// simulate source increment for each entity
			if (1 == sequenceId)
				statementBuilder.setSource({ statementBuilder.source().PrimaryId + 1, 0 });

			// add receipt breadcrumb
			model::Receipt receipt{};
			receipt.Size = sizeof(model::Receipt);
			receipt.Type = static_cast<model::ReceiptType>(2 * breadcrumbId);
			statementBuilder.addReceipt(receipt);
		}

	private:
		std::string m_name;
		bool m_enableRollbackEmulation;
		bool m_enableReceiptGeneration;
	};

	// endregion

	// region MockAggregateNotificationValidator

	struct StatefulValidateParams {
	public:
		StatefulValidateParams(const MockNotification& notification, const validators::ValidatorContext& context)
				: HashCopy(notification.Hash)
				, SequenceId(notification.Id)
				, Context(context)
				, IsPassedMarkedCache(IsMarkedCache(context.Cache))
				, NumStatistics(context.Cache.sub<cache::BlockStatisticCache>().size())
		{}

	public:
		const Hash256 HashCopy;
		const size_t SequenceId;
		const validators::ValidatorContext Context;
		const bool IsPassedMarkedCache;
		const size_t NumStatistics;
	};

	class MockAggregateNotificationValidator
			: public validators::stateful::AggregateNotificationValidator
			, public ParamsCapture<StatefulValidateParams> {
	public:
		MockAggregateNotificationValidator()
				: m_name("MockAggregateNotificationValidator")
				, m_result(validators::ValidationResult::Success)
				, m_numValidateCalls(0)
				, m_validateTrigger(0)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		std::vector<std::string> names() const override {
			return { name() };
		}

		validators::ValidationResult validate(
				const model::Notification& notification,
				const validators::ValidatorContext& context) const override {
			if (MockNotification::Notification_Type != notification.Type)
				return validators::ValidationResult::Success;

			const auto& mockNotification = CastToDerivedNotification<MockNotification>(notification);
			const_cast<MockAggregateNotificationValidator*>(this)->push(mockNotification, context);

			// - determine the result based on the call count
			auto result = ++m_numValidateCalls < m_validateTrigger ? validators::ValidationResult::Success : m_result;

			// - determine the result based on the hash
			auto resultIter = m_hashResults.find(mockNotification.Hash);
			auto idTriggerIter = m_hashIdTriggers.find(mockNotification.Hash);
			return m_hashResults.cend() == resultIter || idTriggerIter->second != mockNotification.Id ? result : resultIter->second;
		}

	public:
		/// Sets the result of validate to \a result after \a trigger calls.
		void setResult(validators::ValidationResult result, size_t trigger = 0) {
			m_result = result;
			m_validateTrigger = trigger;
		}

		/// Sets the result of validate for \a hash with sequence \a id to \a result.
		void setResult(validators::ValidationResult result, const Hash256& hash, size_t id) {
			m_hashResults.emplace(hash, result);
			m_hashIdTriggers.emplace(hash, id);
		}

	private:
		std::string m_name;
		validators::ValidationResult m_result;
		mutable size_t m_numValidateCalls;
		size_t m_validateTrigger;
		std::unordered_map<Hash256, validators::ValidationResult, utils::ArrayHasher<Hash256>> m_hashResults;
		std::unordered_map<Hash256, size_t, utils::ArrayHasher<Hash256>> m_hashIdTriggers;
	};

	// endregion

	// region MockExecutionConfiguration

	struct MockExecutionConfiguration {
	public:
		MockExecutionConfiguration()
				: pObserver(std::make_shared<MockAggregateNotificationObserver>())
				, pValidator(std::make_shared<MockAggregateNotificationValidator>())
				, pNotificationPublisher(std::make_shared<MockNotificationPublisher>()) {
			Config.Network.Identifier = Mock_Execution_Configuration_Network_Identifier;
			Config.pObserver = pObserver;
			Config.pValidator = pValidator;
			Config.pNotificationPublisher = pNotificationPublisher;

			Config.ResolverContextFactory = [](const auto& cache) {
				// 1. use custom mosaic resolver that is dependent on cache parameter
				// 2. use default address resolver
				auto isMarkedCache = IsMarkedCache(cache);
				return model::ResolverContext(
						[isMarkedCache](const auto& unresolved) { return MosaicId(unresolved.unwrap() * (isMarkedCache ? 2 : 0)); },
						[](const auto& unresolved) { return model::ResolverContext().resolve(unresolved); });
			};
		}

	public:
		chain::ExecutionConfiguration Config;
		std::shared_ptr<MockAggregateNotificationObserver> pObserver;
		std::shared_ptr<MockAggregateNotificationValidator> pValidator;
		std::shared_ptr<MockNotificationPublisher> pNotificationPublisher;

	public:
		/// Asserts observer contexts passed to \a observer reflect \a numInitialCacheStatistics
		/// given \a expectedHeight, \a expectedImportanceHeight and \a isRollbackExecution.
		static void AssertObserverContexts(
				const MockAggregateNotificationObserver& observer,
				size_t numInitialCacheStatistics,
				Height expectedHeight,
				model::ImportanceHeight expectedImportanceHeight,
				const predicate<size_t>& isRollbackExecution) {
			// Assert:
			size_t i = 0;
			for (const auto& params : observer.params()) {
				auto message = "observer at " + std::to_string(i);

				// - context (use resolver call to implicitly test creation of ResolverContext)
				EXPECT_EQ(expectedHeight, params.Context.Height) << message;
				if (isRollbackExecution(i))
					EXPECT_EQ(observers::NotifyMode::Rollback, params.Context.Mode) << message;
				else
					EXPECT_EQ(observers::NotifyMode::Commit, params.Context.Mode) << message;

				EXPECT_EQ(MosaicId(22), params.Context.Resolvers.resolve(UnresolvedMosaicId(11))) << message;

				// - compare the copied state to the default state
				EXPECT_EQ(expectedImportanceHeight, params.StateCopy.LastRecalculationHeight) << message;

				// - cache contents + sequence (NumStatistics is incremented by each observer call)
				EXPECT_TRUE(params.IsPassedMarkedCache) << message;
				EXPECT_EQ(numInitialCacheStatistics + i, params.NumStatistics) << message;
				++i;
			}
		}

		/// Asserts validator contexts passed to \a validator reflect \a expectedNumStatistics
		/// given \a expectedHeight and \a expectedBlockTime.
		static void AssertValidatorContexts(
				const MockAggregateNotificationValidator& validator,
				const std::vector<size_t>& expectedNumStatistics,
				Height expectedHeight,
				Timestamp expectedBlockTime) {
			// Assert:
			ASSERT_EQ(expectedNumStatistics.size(), validator.params().size());

			size_t i = 0;
			for (const auto& params : validator.params()) {
				auto message = "validator at " + std::to_string(i);

				// - context (use resolver call to implicitly test creation of ResolverContext)
				EXPECT_EQ(expectedHeight, params.Context.Height) << message;
				EXPECT_EQ(expectedBlockTime, params.Context.BlockTime) << message;
				EXPECT_EQ(Mock_Execution_Configuration_Network_Identifier, params.Context.Network.Identifier) << message;
				EXPECT_EQ(MosaicId(22), params.Context.Resolvers.resolve(UnresolvedMosaicId(11))) << message;

				// - cache contents + sequence (NumStatistics is incremented by each observer call)
				EXPECT_TRUE(params.IsPassedMarkedCache) << message;
				EXPECT_EQ(expectedNumStatistics[i], params.NumStatistics) << message;
				++i;
			}
		}
	};

	// endregion
}}

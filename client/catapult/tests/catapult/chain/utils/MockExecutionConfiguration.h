#pragma once
#include "catapult/cache/BlockDifficultyCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/chain/ExecutionConfiguration.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/utils/Hashers.h"
#include "catapult/validators/AggregateEntityValidator.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include <unordered_map>

namespace catapult { namespace chain {

	constexpr auto Mock_Execution_Configuration_Network_Identifier = model::NetworkIdentifier::Mijin_Test;

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
		explicit MockNotification(const Hash256& hash, size_t id)
				: Notification(static_cast<model::NotificationType>(-1), sizeof(MockNotification))
				, Hash(hash)
				, Id(id)
		{}

	public:
		const Hash256 Hash;
		size_t Id;
	};

	class MockNotificationPublisher : public model::NotificationPublisher, public test::ParamsCapture<PublisherParams> {
	public:
		void publish(const model::WeakEntityInfo& entityInfo, model::NotificationSubscriber& subscriber) const override {
			const_cast<MockNotificationPublisher*>(this)->push(entityInfo);
			subscriber.notify(MockNotification(entityInfo.hash(), 1));
			subscriber.notify(MockNotification(entityInfo.hash(), 2));
		}
	};

	// endregion

	// region MockAggregateNotificationObserver

	struct ObserverParams {
	public:
		explicit ObserverParams(
				const MockNotification& notification,
				const observers::ObserverContext& context)
				: HashCopy(notification.Hash)
				, SequenceId(notification.Id)
				, Context(context)
				, IsPassedMarkedCache(test::IsMarkedCache(context.Cache))
				, NumDifficultyInfos(context.Cache.sub<cache::BlockDifficultyCache>().size())
				, StateCopy(context.State) // make a copy of the state
		{}

	public:
		const Hash256 HashCopy;
		const size_t SequenceId;
		const observers::ObserverContext Context;
		const bool IsPassedMarkedCache;
		const size_t NumDifficultyInfos;
		const state::CatapultState StateCopy;
	};

	class MockAggregateNotificationObserver : public observers::AggregateNotificationObserver, public test::ParamsCapture<ObserverParams> {
	public:
		MockAggregateNotificationObserver() : m_name("MockAggregateNotificationObserver")
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		std::vector<std::string> names() const override {
			return { name() };
		}

		void notify(
				const model::Notification& notification,
				const observers::ObserverContext& context) const override {
			const_cast<MockAggregateNotificationObserver*>(this)->push(
					test::CastToDerivedNotification<MockNotification>(notification),
					context);

			// - add a block difficulty info to the cache as a marker
			auto& cache = context.Cache.sub<cache::BlockDifficultyCache>();
			cache.insert(state::BlockDifficultyInfo(Height(cache.size() + 1)));
		}

	private:
		std::string m_name;
	};

	// endregion

	// region MockAggregateNotificationValidator

	struct StatefulValidateParams {
	public:
		explicit StatefulValidateParams(const MockNotification& notification, const validators::ValidatorContext& context)
				: HashCopy(notification.Hash)
				, SequenceId(notification.Id)
				, Context(context)
				, IsPassedMarkedCache(test::IsMarkedCache(context.Cache))
				, NumDifficultyInfos(context.Cache.sub<cache::BlockDifficultyCache>().size())
		{}

	public:
		const Hash256 HashCopy;
		const size_t SequenceId;
		const validators::ValidatorContext Context;
		const bool IsPassedMarkedCache;
		const size_t NumDifficultyInfos;
	};

	class MockAggregateNotificationValidator
			: public validators::stateful::AggregateNotificationValidator
			, public test::ParamsCapture<StatefulValidateParams> {
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
			const auto& mockNotification = test::CastToDerivedNotification<MockNotification>(notification);
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
		}

	public:
		ExecutionConfiguration Config;
		std::shared_ptr<MockAggregateNotificationObserver> pObserver;
		std::shared_ptr<MockAggregateNotificationValidator> pValidator;
		std::shared_ptr<MockNotificationPublisher> pNotificationPublisher;
	};
}}

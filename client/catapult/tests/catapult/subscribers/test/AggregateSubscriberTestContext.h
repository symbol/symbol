#pragma once
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/EntityInfo.h"
#include "tests/test/nodeps/Pointers.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// A test context for an aggregate subscriber.
	template<typename TSubscriber, typename TAggregate>
	class AggregateSubscriberTestContext {
	public:
		/// Creates a test context around an aggregate with \a numSubscribers subscribers.
		explicit AggregateSubscriberTestContext(size_t numSubscribers = 3)
				: m_subscribers(test::CreatePointers<TSubscriber>(numSubscribers))
				, m_rawSubscribers(test::GetRawPointers(m_subscribers))
				, m_aggregate(std::move(m_subscribers))
		{}

	public:
		/// Gets the subscribers.
		auto& subscribers() const {
			return m_rawSubscribers;
		}

		/// Gets the aggregate.
		auto& aggregate() {
			return m_aggregate;
		}

	private:
		std::vector<std::unique_ptr<TSubscriber>> m_subscribers; // notice that this is moved into m_aggregate
		std::vector<TSubscriber*> m_rawSubscribers;
		TAggregate m_aggregate;
	};

/// Defines a mock for \a SUBSCRIBER_NAME that captures infos from \a FUNC_NAME.
#define DEFINE_MOCK_INFOS_CAPTURE(SUBSCRIBER_NAME, FUNC_NAME) \
	class Mock##SUBSCRIBER_NAME : public Unsupported##SUBSCRIBER_NAME { \
	public: \
		std::vector<const TransactionInfos*> CapturedInfos; \
	\
	public: \
		void FUNC_NAME(const TransactionInfos& transactionInfos) override { \
			CapturedInfos.push_back(&transactionInfos); \
		} \
	}

/// Defines a mock for \a SUBSCRIBER_NAME that captures flushes.
#define DEFINE_MOCK_FLUSH_CAPTURE(SUBSCRIBER_NAME) \
	class Mock##SUBSCRIBER_NAME : public Unsupported##SUBSCRIBER_NAME { \
	public: \
		size_t NumFlushes = 0; \
	\
	public: \
		void flush() override { \
			++NumFlushes; \
		} \
	}

	/// Asserts all subscribers in \a context were called with transaction infos (\a transactionInfos).
	template<typename TTestContext>
	void AssertInfosDelegation(const TTestContext& context, const model::TransactionInfosSet& transactionInfos) {
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			ASSERT_EQ(1u, pSubscriber->CapturedInfos.size()) << message;
			EXPECT_EQ(&transactionInfos, pSubscriber->CapturedInfos[0]) << message;
		}
	}

	/// Asserts all subscribers in \a context were flushed exactly once.
	template<typename TTestContext>
	void AssertFlushDelegation(const TTestContext& context) {
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			EXPECT_EQ(1u, pSubscriber->NumFlushes) << message;
		}
	}
}}

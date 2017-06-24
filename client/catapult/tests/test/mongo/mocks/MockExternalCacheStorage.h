#pragma once
#include "plugins/mongo/coremongo/src/ExternalCacheStorage.h"
#include "tests/test/cache/SimpleCache.h"

namespace catapult { namespace mocks {

	/// Simple mock external cache storage implementation.
	template<size_t CacheId>
	class MockExternalCacheStorage final : public mongo::plugins::ExternalCacheStorageT<test::SimpleCacheT<CacheId>> {
	private:
		using LoadCheckpointFunc = typename mongo::plugins::ExternalCacheStorageT<test::SimpleCacheT<CacheId>>::LoadCheckpointFunc;

	public:
		/// Creates a mock external cache storage.
		MockExternalCacheStorage()
				: m_numSaveDeltaCalls(0)
				, m_numLoadAllCalls(0)
		{}

	private:
		void saveDelta(const test::SimpleCacheDelta&) override {
			++m_numSaveDeltaCalls;
		}

		void loadAll(test::SimpleCacheDelta&, Height chainHeight, const LoadCheckpointFunc&) const override {
			++m_numLoadAllCalls;
			m_chainHeight = chainHeight;
		}

	public:
		/// Gets the number of save delta calls.
		size_t numSaveDeltaCalls() const {
			return m_numSaveDeltaCalls;
		}

		/// Gets the number of load all calls.
		size_t numLoadAllCalls() const {
			return m_numLoadAllCalls;
		}

		/// Gets the last chain height seen in load all.
		Height chainHeight() const {
			return m_chainHeight;
		}

	private:
		size_t m_numSaveDeltaCalls;
		mutable size_t m_numLoadAllCalls;
		mutable Height m_chainHeight;
	};
}}

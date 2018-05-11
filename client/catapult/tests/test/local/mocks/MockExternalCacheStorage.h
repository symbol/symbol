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
#include "extensions/mongo/src/ExternalCacheStorage.h"
#include "tests/test/cache/SimpleCache.h"

namespace catapult { namespace mocks {

	/// Simple mock external cache storage implementation.
	template<size_t CacheId>
	class MockExternalCacheStorage final : public mongo::ExternalCacheStorageT<test::SimpleCacheT<CacheId>> {
	private:
		using LoadCheckpointFunc = typename mongo::ExternalCacheStorageT<test::SimpleCacheT<CacheId>>::LoadCheckpointFunc;

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

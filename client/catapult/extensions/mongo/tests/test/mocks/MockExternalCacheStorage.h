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
#include "extensions/mongo/src/ExternalCacheStorage.h"
#include "tests/test/cache/SimpleCache.h"

namespace catapult { namespace mocks {

	/// Simple mock external cache storage implementation.
	template<size_t CacheId>
	class MockExternalCacheStorage final : public mongo::ExternalCacheStorageT<test::SimpleCacheT<CacheId>> {
	public:
		/// Creates a mock external cache storage.
		MockExternalCacheStorage() : m_numSaveDeltaCalls(0)
		{}

	private:
		void saveDelta(const cache::SingleCacheChangesT<test::SimpleCacheDelta, uint64_t>&) override {
			++m_numSaveDeltaCalls;
		}

	public:
		/// Gets the number of save delta calls.
		size_t numSaveDeltaCalls() const {
			return m_numSaveDeltaCalls;
		}

		/// Gets the last chain height seen in load all.
		Height chainHeight() const {
			return m_chainHeight;
		}

	private:
		size_t m_numSaveDeltaCalls;
		mutable Height m_chainHeight;
	};
}}

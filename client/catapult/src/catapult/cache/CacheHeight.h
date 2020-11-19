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
#include "catapult/utils/SpinReaderWriterLock.h"
#include "catapult/types.h"

namespace catapult { namespace cache {

	/// Read only view on top of a cache height.
	class CacheHeightView : public utils::MoveOnly {
	public:
		/// Creates a cache height view around \a height with lock context \a readLock.
		CacheHeightView(Height height, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
				: m_height(height)
				, m_readLock(std::move(readLock))
		{}

	public:
		/// Gets the height.
		Height get() const {
			return m_height;
		}

	private:
		Height m_height;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// Write only view on top of a cache height.
	class CacheHeightModifier : public utils::MoveOnly {
	public:
		/// Creates a write only view around \a height with lock context \a writeLock.
		CacheHeightModifier(Height& height, utils::SpinReaderWriterLock::WriterLockGuard&& writeLock)
				: m_height(height)
				, m_writeLock(std::move(writeLock))
		{}

	public:
		/// Sets the cache height to \a height.
		void set(Height height) {
			m_height = height;
		}

	private:
		Height& m_height;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	/// Synchronized height associated with a catapult cache.
	class CacheHeight {
	public:
		/// Gets a read only view of the height.
		CacheHeightView view() const {
			auto readLock = m_lock.acquireReader();
			return CacheHeightView(m_height, std::move(readLock));
		}

		/// Gets a write only view of the height.
		CacheHeightModifier modifier() {
			auto writeLock = m_lock.acquireWriter();
			return CacheHeightModifier(m_height, std::move(writeLock));
		}

	private:
		Height m_height;
		mutable utils::SpinReaderWriterLock m_lock;
	};
}}

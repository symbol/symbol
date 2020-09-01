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
#include "ProofStorage.h"
#include "catapult/utils/SpinReaderWriterLock.h"

namespace catapult { namespace io {

	/// Read only view on top of proof storage.
	class ProofStorageView : utils::MoveOnly {
	public:
		/// Creates a view around \a storage with lock context \a readLock.
		ProofStorageView(const ProofStorage& storage, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Gets the statistics of the last finalized block.
		model::FinalizationStatistics statistics() const;

		/// Gets the finalization proof at \a point.
		std::shared_ptr<const model::FinalizationProof> loadProof(FinalizationPoint point) const;

		/// Gets the first finalization proof at \a height.
		std::shared_ptr<const model::FinalizationProof> loadProof(Height height) const;

	private:
		const ProofStorage& m_storage;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// Write only view on top of proof storage.
	class ProofStorageModifier : utils::MoveOnly {
	public:
		/// Creates a view around \a storage with lock context \a writeLock.
		ProofStorageModifier(ProofStorage& storage, utils::SpinReaderWriterLock::WriterLockGuard&& writeLock);

	public:
		/// Saves finalization \a proof.
		void saveProof(const model::FinalizationProof& proof);

	private:
		ProofStorage& m_storage;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	/// Cache around a ProofStorage.
	/// \note Currently this "cache" provides synchronization.
	class ProofStorageCache {
	public:
		/// Creates a new cache around \a pStorage.
		explicit ProofStorageCache(std::unique_ptr<ProofStorage>&& pStorage);

		/// Destroys the cache.
		~ProofStorageCache();

	public:
		/// Gets a read only view of the storage.
		ProofStorageView view() const;

		/// Gets a write only view of the storage.
		ProofStorageModifier modifier();

	private:
		std::unique_ptr<ProofStorage> m_pStorage;
		mutable utils::SpinReaderWriterLock m_lock;
	};
}}

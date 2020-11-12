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
#include "finalization/src/io/ProofStorage.h"

namespace catapult { namespace mocks {

	/// Mock proof storage.
	class MockProofStorage : public io::ProofStorage {
	public:
		/// Creates a proof storage initialized with a nemesis proof.
		MockProofStorage() : MockProofStorage(FinalizationPoint(1), Height(1))
		{}

		/// Creates a proof storage initialized with a proof for \a point and \a height.
		MockProofStorage(FinalizationPoint point, Height height) : MockProofStorage(point, height, Hash256())
		{}

		/// Creates a proof storage initialized with a proof for \a point, \a height and \a hash.
		MockProofStorage(FinalizationPoint point, Height height, const Hash256& hash)
				: MockProofStorage(FinalizationEpoch(1), point, height, hash)
		{}

		/// Creates a proof storage initialized with a proof for \a epoch, \a point, \a height and \a hash.
		MockProofStorage(FinalizationEpoch epoch, FinalizationPoint point, Height height, const Hash256& hash) {
			setLastFinalization(epoch, point, height, hash);
		}

	public:
		/// Gets all saved proof descriptors.
		const auto& savedProofDescriptors() const {
			return m_savedProofDescriptors;
		}

	public:
		/// Sets the last finalization \a point and \a height.
		void setLastFinalization(FinalizationPoint point, Height height) {
			setLastFinalization(point, height, Hash256());
		}

		/// Sets the last finalization \a point, \a height and \a hash.
		void setLastFinalization(FinalizationPoint point, Height height, const Hash256& hash) {
			setLastFinalization(FinalizationEpoch(1), point, height, hash);
		}

		/// Sets the last finalization \a epoch, \a point, \a height and \a hash.
		void setLastFinalization(FinalizationEpoch epoch, FinalizationPoint point, Height height, const Hash256& hash) {
			m_epoch = epoch;
			m_point = point;
			m_height = height;
			m_hash = hash;
		}

		/// Sets the last finalization proof to \a pProof.
		void setLastFinalizationProof(const std::shared_ptr<const model::FinalizationProof>& pProof) {
			m_proofs.push_back(pProof);
		}

	public:
		model::FinalizationStatistics statistics() const override {
			return { { m_epoch, m_point }, m_height, m_hash };
		}

		std::shared_ptr<const model::FinalizationProof> loadProof(FinalizationEpoch epoch) const override {
			if (FinalizationEpoch() == epoch || epoch > m_epoch)
				CATAPULT_THROW_INVALID_ARGUMENT("epoch must be nonzero and no greater than finalizationPoint");

			auto iter = std::find_if(m_proofs.crbegin(), m_proofs.crend(), [epoch](const auto& pProof) {
				return epoch == pProof->Round.Epoch;
			});
			return m_proofs.crend() == iter ? nullptr : *iter;
		}

		std::shared_ptr<const model::FinalizationProof> loadProof(Height height) const override {
			if (Height() == height || height > m_height)
				CATAPULT_THROW_INVALID_ARGUMENT("height must be nonzero and no greater than finalizedHeight");

			auto iter = std::find_if(m_proofs.crbegin(), m_proofs.crend(), [height](const auto& pProof) {
				return height == pProof->Height;
			});
			return m_proofs.crend() == iter ? nullptr : *iter;
		}

		void saveProof(const model::FinalizationProof& proof) override {
			m_epoch = proof.Round.Epoch;
			m_point = proof.Round.Point;
			m_height = proof.Height;
			m_hash = proof.Hash;

			m_savedProofDescriptors.push_back(statistics());
		}

	private:
		FinalizationEpoch m_epoch;
		FinalizationPoint m_point;
		Height m_height;
		Hash256 m_hash;

		std::vector<std::shared_ptr<const model::FinalizationProof>> m_proofs;
		std::vector<model::FinalizationStatistics> m_savedProofDescriptors;
	};
}}

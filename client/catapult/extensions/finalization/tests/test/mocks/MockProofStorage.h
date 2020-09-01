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
		MockProofStorage(FinalizationPoint point, Height height, const Hash256& hash) {
			setLastFinalization(point, height, hash);
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
			m_point = point;
			m_height = height;
			m_hash = hash;
		}

		/// Sets the last finalization proof to \a pProof.
		void setLastFinalizationProof(const std::shared_ptr<const model::FinalizationProof>& pProof) {
			m_pProof = pProof;
		}

	public:
		model::FinalizationStatistics statistics() const override {
			return { m_point, m_height, m_hash };
		}

		std::shared_ptr<const model::FinalizationProof> loadProof(FinalizationPoint point) const override {
			if (FinalizationPoint() == point || point > m_point)
				CATAPULT_THROW_INVALID_ARGUMENT("point must be nonzero and no greater than finalizationPoint");

			return m_point == point ? m_pProof : nullptr;
		}

		std::shared_ptr<const model::FinalizationProof> loadProof(Height height) const override {
			if (Height() == height || height > m_height)
				CATAPULT_THROW_INVALID_ARGUMENT("height must be nonzero and no greater than finalizedHeight");

			return m_height == height ? m_pProof : nullptr;
		}

		void saveProof(const model::FinalizationProof& proof) override {
			m_point = proof.Point;
			m_height = proof.Height;
			m_hash = proof.Hash;

			m_savedProofDescriptors.push_back(model::FinalizationStatistics{ m_point, m_height, m_hash });
		}

	private:
		FinalizationPoint m_point;
		Height m_height;
		Hash256 m_hash;

		std::shared_ptr<const model::FinalizationProof> m_pProof;
		std::vector<model::FinalizationStatistics> m_savedProofDescriptors;
	};
}}

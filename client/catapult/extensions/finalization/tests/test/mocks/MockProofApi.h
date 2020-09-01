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
#include "finalization/src/api/RemoteProofApi.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace mocks {

	/// Mock proof api that can be configured to return predefined data for requests, capture function parameters
	/// and throw exceptions at specified entry points.
	class MockProofApi : public api::RemoteProofApi {
	public:
		enum class EntryPoint {
			None,
			Finalization_Statistics,
			Proof_At_Point,
			Proof_At_Height
		};

	public:
		/// Creates a proof api.
		MockProofApi()
				: api::RemoteProofApi({ test::GenerateRandomByteArray<Key>(), "fake-host-from-mock-proof-api" })
				, m_errorEntryPoint(EntryPoint::None)
		{}

	public:
		/// Sets the entry point where the exception should occur to \a entryPoint.
		void setError(EntryPoint entryPoint) {
			m_errorEntryPoint = entryPoint;
		}

		/// Gets a vector of parameters that were passed to the proof at point requests.
		const auto& proofPoints() const {
			return m_proofPoints;
		}

		/// Gets a vector of parameters that were passed to the proof at height requests.
		const auto& proofHeights() const {
			return m_proofHeights;
		}

	public:
		/// Sets the finalization statistics to \a finalizationStatistics.
		void setFinalizationStatistics(const model::FinalizationStatistics& finalizationStatistics) {
			m_finalizationStatistics = finalizationStatistics;
		}

		/// Sets the finalization proof to \a pProof.
		void setProof(const std::shared_ptr<const model::FinalizationProof>& pProof) {
			m_pProof = pProof;
		}

	public:
		/// Gets the configured statistics and throws if the error entry point is set to Finalization_Statistics.
		thread::future<model::FinalizationStatistics> finalizationStatistics() const override {
			if (shouldRaiseException(EntryPoint::Finalization_Statistics))
				return CreateFutureException<model::FinalizationStatistics>("finalization statistics error has been set");

			return thread::make_ready_future(decltype(m_finalizationStatistics)(m_finalizationStatistics));
		}

		/// Gets the configured proof and throws if the error entry point is set to Proof_At_Point.
		/// \note The \a point parameter is captured.
		thread::future<std::shared_ptr<const model::FinalizationProof>> proofAt(FinalizationPoint point) const override {
			m_proofPoints.push_back(point);
			if (shouldRaiseException(EntryPoint::Proof_At_Point))
				return CreateFutureException<std::shared_ptr<const model::FinalizationProof>>("proof at point error has been set");

			return thread::make_ready_future(decltype(m_pProof)(m_pProof));
		}

		/// Gets the configured proof and throws if the error entry point is set to Proof_At_Height.
		/// \note The \a height parameter is captured.
		thread::future<std::shared_ptr<const model::FinalizationProof>> proofAt(Height height) const override {
			m_proofHeights.push_back(height);
			if (shouldRaiseException(EntryPoint::Proof_At_Height))
				return CreateFutureException<std::shared_ptr<const model::FinalizationProof>>("proof at height error has been set");

			return thread::make_ready_future(decltype(m_pProof)(m_pProof));
		}

	private:
		bool shouldRaiseException(EntryPoint entryPoint) const {
			return m_errorEntryPoint == entryPoint;
		}

		template<typename T>
		static thread::future<T> CreateFutureException(const char* message) {
			return thread::make_exceptional_future<T>(catapult_runtime_error(message));
		}

	private:
		model::FinalizationStatistics m_finalizationStatistics;
		std::shared_ptr<const model::FinalizationProof> m_pProof;

		EntryPoint m_errorEntryPoint;
		mutable std::vector<FinalizationPoint> m_proofPoints;
		mutable std::vector<Height> m_proofHeights;
	};
}}

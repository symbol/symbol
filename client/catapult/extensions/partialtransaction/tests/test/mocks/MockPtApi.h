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
#include "partialtransaction/src/api/RemotePtApi.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace mocks {

	/// Mock partial transaction api that can be configured to return predefined data for requests, capture function parameters
	/// and throw exceptions at specified entry points.
	class MockPtApi : public api::RemotePtApi {
	public:
		enum class EntryPoint {
			None,
			Partial_Transaction_Infos
		};

	public:
		/// Creates a partial transaction api around cosigned transaction infos (\a transactionInfos).
		explicit MockPtApi(const partialtransaction::CosignedTransactionInfos& transactionInfos)
				: api::RemotePtApi({ test::GenerateRandomByteArray<Key>(), "fake-host-from-mock-pt-api" })
				, m_transactionInfos(transactionInfos)
				, m_errorEntryPoint(EntryPoint::None)
		{}

	public:
		/// Sets the entry point where the exception should occur to \a entryPoint.
		void setError(EntryPoint entryPoint) {
			m_errorEntryPoint = entryPoint;
		}

		/// Gets a vector of short hash pair ranges that were passed to the partial transaction infos requests.
		const std::vector<cache::ShortHashPairRange>& transactionInfosRequests() const {
			return m_transactionInfosRequests;
		}

	public:
		/// Gets the configured partial transaction infos and throws if the error entry point is set to Partial_Transaction_Infos.
		/// \note The \a knownShortHashPairs parameter is captured.
		thread::future<partialtransaction::CosignedTransactionInfos> transactionInfos(
				cache::ShortHashPairRange&& knownShortHashPairs) const override {
			m_transactionInfosRequests.push_back(std::move(knownShortHashPairs));
			if (shouldRaiseException(EntryPoint::Partial_Transaction_Infos)) {
				using ResultType = partialtransaction::CosignedTransactionInfos;
				return CreateFutureException<ResultType>("partial transaction infos error has been set");
			}

			return thread::make_ready_future(decltype(m_transactionInfos)(m_transactionInfos));
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
		partialtransaction::CosignedTransactionInfos m_transactionInfos;
		EntryPoint m_errorEntryPoint;
		mutable std::vector<cache::ShortHashPairRange> m_transactionInfosRequests;
	};
}}

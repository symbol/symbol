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
#include "catapult/api/RemoteTransactionApi.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace mocks {

	/// Mock transaction api that can be configured to return predefined data for requests, capture function parameters
	/// and throw exceptions at specified entry points.
	class MockTransactionApi : public api::RemoteTransactionApi {
	public:
		enum class EntryPoint {
			None,
			Unconfirmed_Transactions
		};

	public:
		/// Creates a transaction api around a range of transactions (\a transactionRange).
		explicit MockTransactionApi(const model::TransactionRange& transactionRange)
				: api::RemoteTransactionApi({ test::GenerateRandomByteArray<Key>(), "fake-host-from-mock-transaction-api" })
				, m_transactionRange(model::TransactionRange::CopyRange(transactionRange))
				, m_errorEntryPoint(EntryPoint::None)
		{}

	public:
		/// Sets the entry point where the exception should occur to \a entryPoint.
		void setError(EntryPoint entryPoint) {
			m_errorEntryPoint = entryPoint;
		}

		/// Gets a vector of parameters that were passed to the unconfirmed transactions requests.
		const auto& utRequests() const {
			return m_utRequests;
		}

	public:
		/// Gets the configured unconfirmed transactions and throws if the error entry point is set to Unconfirmed_Transactions.
		/// \note The \a minFeeMultiplier and \a knownShortHashes parameters are captured.
		thread::future<model::TransactionRange> unconfirmedTransactions(
				BlockFeeMultiplier minFeeMultiplier,
				model::ShortHashRange&& knownShortHashes) const override {
			m_utRequests.emplace_back(minFeeMultiplier, std::move(knownShortHashes));
			if (shouldRaiseException(EntryPoint::Unconfirmed_Transactions))
				return CreateFutureException<model::TransactionRange>("unconfirmed transactions error has been set");

			return thread::make_ready_future(model::TransactionRange::CopyRange(m_transactionRange));
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
		model::TransactionRange m_transactionRange;
		EntryPoint m_errorEntryPoint;
		mutable std::vector<std::pair<BlockFeeMultiplier, model::ShortHashRange>> m_utRequests;
	};
}}

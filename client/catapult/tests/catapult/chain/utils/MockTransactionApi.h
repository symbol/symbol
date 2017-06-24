#pragma once
#include "catapult/api/RemoteTransactionApi.h"

namespace catapult { namespace mocks {

	/// A mock transaction api that can be configured to return predefined data for requests, capture function parameters
	/// and throw exceptions at specified entry points.
	class MockTransactionApi : public api::RemoteTransactionApi {
	public:
		enum class EntryPoint {
			None,
			Unconfirmed_Transactions
		};

	public:
		/// Creates a transaction api around a range of \a transactions.
		explicit MockTransactionApi(const model::TransactionRange& transactions)
				: m_transactions(model::TransactionRange::CopyRange(transactions))
				, m_errorEntryPoint(EntryPoint::None)
		{}

	public:
		/// Sets the entry point where the exception should occur to \a entryPoint.
		void setError(EntryPoint entryPoint) {
			m_errorEntryPoint = entryPoint;
		}

		/// Returns the vector of short hash ranges that were passed to the unconfirmed-transactions requests.
		const std::vector<model::ShortHashRange>& unconfirmedTransactionsRequests() const {
			return m_unconfirmedTransactionsRequests;
		}

	public:
		/// Returns the configured unconfirmed transactions, throws if the error entry point is set to Unconfirmed_Transactions.
		thread::future<model::TransactionRange> unconfirmedTransactions(model::ShortHashRange&& shortHashes) const override {
			m_unconfirmedTransactionsRequests.push_back(std::move(shortHashes));
			if (shouldRaiseException(EntryPoint::Unconfirmed_Transactions))
				return CreateFutureException<model::TransactionRange>("unconfirmed transactions error has been set");

			return thread::make_ready_future(model::TransactionRange::CopyRange(m_transactions));
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
		model::TransactionRange m_transactions;
		EntryPoint m_errorEntryPoint;
		mutable std::vector<model::ShortHashRange> m_unconfirmedTransactionsRequests;
	};
}}

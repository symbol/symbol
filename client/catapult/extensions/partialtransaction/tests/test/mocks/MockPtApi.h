#pragma once
#include "partialtransaction/src/api/RemotePtApi.h"

namespace catapult { namespace mocks {

	/// A mock partial transaction api that can be configured to return predefined data for requests, capture function parameters
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
				: m_transactionInfos(transactionInfos)
				, m_errorEntryPoint(EntryPoint::None)
		{}

	public:
		/// Sets the entry point where the exception should occur to \a entryPoint.
		void setError(EntryPoint entryPoint) {
			m_errorEntryPoint = entryPoint;
		}

		/// Returns the vector of short hash pair ranges that were passed to the partial transaction infos requests.
		const std::vector<cache::ShortHashPairRange>& transactionInfosRequests() const {
			return m_transactionInfosRequests;
		}

	public:
		/// Returns the configured partial transaction infos and throws if the error entry point is set to Partial_Transaction_Infos.
		/// The \a knownShortHashPairs parameter is captured.
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

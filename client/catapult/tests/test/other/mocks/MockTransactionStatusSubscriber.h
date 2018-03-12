#pragma once
#include "catapult/model/Transaction.h"
#include "catapult/model/TransactionStatus.h"
#include "catapult/subscribers/TransactionStatusSubscriber.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include <atomic>

namespace catapult { namespace mocks {

	/// Transaction status subscriber status params.
	struct TransactionStatusSubscriberStatusParams {
	public:
		/// Creates params around \a transaction, \a hash and \a status.
		explicit TransactionStatusSubscriberStatusParams(const model::Transaction& transaction, const Hash256& hash, uint32_t status)
				: Transaction(transaction)
				, Hash(hash)
				, HashCopy(hash)
				, Status(status)
		{}

	public:
		/// The transaction.
		const model::Transaction& Transaction;

		/// The transaction hash.
		const Hash256& Hash;

		/// A copy of the transaction hash.
		Hash256 HashCopy;

		/// The transaction status.
		uint32_t Status;
	};

	/// Mock noop transaction status subscriber implementation.
	class MockTransactionStatusSubscriber
			: public subscribers::TransactionStatusSubscriber
			, public test::ParamsCapture<TransactionStatusSubscriberStatusParams> {
	public:
		/// Creates a subscriber.
		MockTransactionStatusSubscriber()
				: m_numNotifies(0)
				, m_numFlushes(0)
		{}

	public:
		/// Gets the number of notifyStatus calls.
		size_t numNotifies() const {
			return m_numNotifies;
		}

		/// Gets the number of flush calls.
		size_t numFlushes() const {
			return m_numFlushes;
		}

	public:
		void notifyStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) override {
			push(transaction, hash, status);
			++m_numNotifies;
		}

		void flush() override {
			++m_numFlushes;
		}

	private:
		std::atomic<size_t> m_numNotifies;
		std::atomic<size_t> m_numFlushes;
	};
}}

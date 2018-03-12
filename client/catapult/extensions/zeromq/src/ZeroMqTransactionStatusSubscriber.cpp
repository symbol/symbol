#include "ZeroMqTransactionStatusSubscriber.h"
#include "ZeroMqEntityPublisher.h"
#include "catapult/model/Transaction.h"
#include "catapult/model/TransactionStatus.h"

namespace catapult { namespace zeromq {

	namespace {
		class ZeroMqTransactionStatusSubscriber : public subscribers::TransactionStatusSubscriber {
		public:
			explicit ZeroMqTransactionStatusSubscriber(ZeroMqEntityPublisher& publisher) : m_publisher(publisher)
			{}

		public:
			void notifyStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) override {
				m_publisher.publishTransactionStatus(transaction, hash, status);
			}

			void flush() override {
				// empty since the publisher will flush all pending statuses periodically
			}

		private:
			ZeroMqEntityPublisher& m_publisher;
		};
	}

	std::unique_ptr<subscribers::TransactionStatusSubscriber> CreateZeroMqTransactionStatusSubscriber(ZeroMqEntityPublisher& publisher) {
		return std::make_unique<ZeroMqTransactionStatusSubscriber>(publisher);
	}
}}

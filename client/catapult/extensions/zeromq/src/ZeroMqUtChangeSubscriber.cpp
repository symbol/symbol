#include "ZeroMqUtChangeSubscriber.h"
#include "ZeroMqEntityPublisher.h"

namespace catapult { namespace zeromq {

	namespace {
		class ZeroMqUtChangeSubscriber : public cache::UtChangeSubscriber {
		public:
			explicit ZeroMqUtChangeSubscriber(ZeroMqEntityPublisher& publisher) : m_publisher(publisher)
			{}

		public:
			void notifyAdds(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_publisher.publishTransaction(TransactionMarker::Unconfirmed_Transaction_Add_Marker, transactionInfo, Height());
			}

			void notifyRemoves(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_publisher.publishTransactionHash(TransactionMarker::Unconfirmed_Transaction_Remove_Marker, transactionInfo);
			}

			void flush() override {
				// empty because messages are pushed by other calls
			}

		private:
			ZeroMqEntityPublisher& m_publisher;
		};
	}

	std::unique_ptr<cache::UtChangeSubscriber> CreateZeroMqUtChangeSubscriber(ZeroMqEntityPublisher& publisher) {
		return std::make_unique<ZeroMqUtChangeSubscriber>(publisher);
	}
}}

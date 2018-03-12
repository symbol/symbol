#include "ZeroMqBlockChangeSubscriber.h"
#include "ZeroMqEntityPublisher.h"
#include "catapult/model/Elements.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace zeromq {

	namespace {
		class ZeroMqBlockChangeSubscriber : public io::BlockChangeSubscriber {
		public:
			explicit ZeroMqBlockChangeSubscriber(ZeroMqEntityPublisher& publisher) : m_publisher(publisher)
			{}

		public:
			void notifyBlock(const model::BlockElement& blockElement) override {
				// block header
				m_publisher.publishBlockHeader(blockElement);

				// transactions
				auto height = blockElement.Block.Height;
				for (const auto& transactionElement : blockElement.Transactions)
					m_publisher.publishTransaction(TransactionMarker::Transaction_Marker, transactionElement, height);
			}

			void notifyDropBlocksAfter(Height height) override {
				m_publisher.publishDropBlocks(height);
			}

		private:
			ZeroMqEntityPublisher& m_publisher;
		};
	}

	std::unique_ptr<io::BlockChangeSubscriber> CreateZeroMqBlockChangeSubscriber(ZeroMqEntityPublisher& publisher) {
		return std::make_unique<ZeroMqBlockChangeSubscriber>(publisher);
	}
}}

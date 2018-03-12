#pragma once
#include "catapult/subscribers/NodeSubscriber.h"
#include "tests/test/nodeps/ParamsCapture.h"

namespace catapult { namespace mocks {

	/// Node subscriber node params.
	struct NodeSubscriberNodeParams {
	public:
		/// Creates params around \a node.
		explicit NodeSubscriberNodeParams(const ionet::Node& node)
				: Node(node)
				, NodeCopy(node)
		{}

	public:
		/// The node.
		const ionet::Node& Node;

		/// A copy of the node.
		const ionet::Node NodeCopy;
	};

	/// Mock noop node subscriber implementation.
	class MockNodeSubscriber : public subscribers::NodeSubscriber, public test::ParamsCapture<NodeSubscriberNodeParams> {
	public:
		void notifyNode(const ionet::Node& node) override {
			push(node);
		}
	};
}}

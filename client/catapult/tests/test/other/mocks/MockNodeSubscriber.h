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
		/// Reference to the node.
		const ionet::Node& Node;

		/// Copy of the node.
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

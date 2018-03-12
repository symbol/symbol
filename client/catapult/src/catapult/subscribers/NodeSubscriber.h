#pragma once

namespace catapult { namespace ionet { class Node; } }

namespace catapult { namespace subscribers {

	/// Node subscriber.
	class NodeSubscriber {
	public:
		virtual ~NodeSubscriber() {}

	public:
		/// Indicates a new \a node was found.
		virtual void notifyNode(const ionet::Node& node) = 0;
	};
}}

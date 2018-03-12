#include "NodeUtils.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "catapult/ionet/NodeContainer.h"

namespace catapult { namespace local {

	namespace {
		void CheckString(const std::string& str, const char* name) {
			if (str.size() <= std::numeric_limits<uint8_t>::max())
				return;

			std::ostringstream out;
			out << name << " is too long (" << str << ")";
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		void ValidateAndAddNode(ionet::NodeContainerModifier& modifier, const ionet::Node& node, ionet::NodeSource source) {
			CheckString(node.endpoint().Host, "host");
			CheckString(node.metadata().Name, "name");
			modifier.add(node, source);
		}
	}

	void SeedNodeContainer(ionet::NodeContainer& nodes, const extensions::LocalNodeBootstrapper& bootstrapper) {
		auto modifier = nodes.modifier();
		for (const auto& node : bootstrapper.staticNodes())
			ValidateAndAddNode(modifier, node, ionet::NodeSource::Static);

		ValidateAndAddNode(modifier, config::ToLocalNode(bootstrapper.config()), ionet::NodeSource::Local);
	}
}}

#pragma once

namespace catapult { namespace extensions {

	/// Ordered enumeration of service registrar phases.
	/// \note Service registrars are invoked in order of their phase from Initial to Terminal.
	enum class ServiceRegistrarPhase {
		/// The first phase (no dependencies) that registers executable modules.
		/// \note This allows services that load modules to be unloaded last.
		Initial_With_Modules,

		/// The first phase (no dependencies).
		Initial,

		/// Requires transaction event handlers to be registered.
		Post_Transaction_Event_Handlers,

		/// Requires remote peers (and entity sinks) to be registered.
		Post_Remote_Peers,

		/// Requires basic range consumers (and factories) to be registered.
		Post_Range_Consumers,

		/// Requires extended range consumers (and factories) to be registered.
		/// \note These are typically configured by extensions.
		Post_Extended_Range_Consumers,

		/// Requires all packet io pickers to be registered.
		Post_Packet_Io_Pickers,

		/// Requires all packet handlers to be registered.
		Post_Packet_Handlers,

		/// Requires all tasks to be registered.
		Post_Tasks
	};
}}

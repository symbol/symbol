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

namespace catapult { namespace extensions {

	/// Ordered enumeration of service registrar phases.
	/// \note Service registrars are invoked in order of their phase from Initial to Terminal.
	enum class ServiceRegistrarPhase {
		/// First phase (no dependencies) that registers executable modules.
		/// \note This allows services that load modules to be unloaded last.
		Initial_With_Modules,

		/// First phase (no dependencies).
		Initial,

		/// Requires transaction event handlers to be registered.
		Post_Transaction_Event_Handlers,

		/// Requires remote peers (and entity sinks) to be registered.
		Post_Remote_Peers,

		/// Requires basic range consumers (and factories) to be registered.
		Post_Range_Consumers,

		/// Requires (phase two) basic range consumers (and factories) to be registered.
		Post_Range_Consumers_Phase_Two,

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

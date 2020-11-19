/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/model/NodeIdentity.h"
#include "catapult/types.h"

namespace catapult { namespace api {

	/// Api for retrieving information from a remote node.
	class RemoteApi {
	protected:
		/// Creates a remote api for the node with specified \a remoteIdentity.
		explicit RemoteApi(const model::NodeIdentity& remoteIdentity) : m_remoteIdentity(remoteIdentity)
		{}

	public:
		virtual ~RemoteApi() = default;

	public:
		/// Gets the remote identity.
		const model::NodeIdentity& remoteIdentity() const {
			return m_remoteIdentity;
		}

	private:
		model::NodeIdentity m_remoteIdentity;
	};
}}

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
#include "catapult/types.h"

namespace catapult { namespace api {

	/// An api for retrieving information from a remote node.
	class RemoteApi {
	protected:
		/// Creates a remote api for the node with specified public key (\a remotePublicKey).
		explicit RemoteApi(const Key& remotePublicKey) : m_remotePublicKey(remotePublicKey)
		{}

	public:
		virtual ~RemoteApi() = default;

	public:
		/// Gets the remote node public key.
		const Key& remotePublicKey() const {
			return m_remotePublicKey;
		}

	private:
		Key m_remotePublicKey;
	};
}}

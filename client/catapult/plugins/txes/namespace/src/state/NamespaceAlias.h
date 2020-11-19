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
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Possible alias types.
	enum class AliasType : uint8_t {
		/// No alias.
		None,

		/// Mosaic id alias.
		Mosaic,

		/// Account address alias.
		Address
	};

	/// Namespace alias.
	class NamespaceAlias {
	public:
		/// Creates an unset namespace alias.
		NamespaceAlias();

		/// Creates a namespace alias around \a mosaicId.
		explicit NamespaceAlias(MosaicId mosaicId);

		/// Creates a namespace alias around \a address.
		explicit NamespaceAlias(const Address& address);

		/// Copy constructor that makes a copy of \a alias.
		NamespaceAlias(const NamespaceAlias& alias);

	public:
		/// Assignment operator that makes a copy of \a alias.
		NamespaceAlias& operator=(const NamespaceAlias& alias);

	public:
		/// Gets the type of alias.
		AliasType type() const;

		/// Gets the mosaic alias.
		MosaicId mosaicId() const;

		/// Gets the address alias.
		const Address& address() const;

	private:
		AliasType m_type;

		union {
			MosaicId m_mosaicId;
			Address m_address;
		};
	};
}}

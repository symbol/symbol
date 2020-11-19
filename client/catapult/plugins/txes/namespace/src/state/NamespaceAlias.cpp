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

#include "NamespaceAlias.h"
#include "catapult/exceptions.h"

namespace catapult { namespace state {

	NamespaceAlias::NamespaceAlias() : m_type(AliasType::None)
	{}

	NamespaceAlias::NamespaceAlias(MosaicId mosaicId)
			: m_type(AliasType::Mosaic)
			, m_mosaicId(mosaicId)
	{}

	NamespaceAlias::NamespaceAlias(const Address& address)
			: m_type(AliasType::Address)
			, m_address(address)
	{}

	NamespaceAlias::NamespaceAlias(const NamespaceAlias& alias)
			: m_type(alias.m_type)
			, m_address(alias.m_address)
	{}

	NamespaceAlias& NamespaceAlias::operator=(const NamespaceAlias& alias) {
		m_type = alias.m_type;
		m_address = alias.m_address;
		return *this;
	}

	AliasType NamespaceAlias::type() const {
		return m_type;
	}

	MosaicId NamespaceAlias::mosaicId() const {
		if (AliasType::Mosaic != m_type)
			CATAPULT_THROW_RUNTIME_ERROR("NamespaceAlias::mosaicId called with invalid type");

		return m_mosaicId;
	}

	const Address& NamespaceAlias::address() const {
		if (AliasType::Address != m_type)
			CATAPULT_THROW_RUNTIME_ERROR("NamespaceAlias::address called with invalid type");

		return m_address;
	}
}}

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
#include "PodIoUtils.h"
#include "catapult/model/SizePrefixedEntity.h"
#include "catapult/utils/MemoryUtils.h"
#include <memory>

namespace catapult { namespace io {

	/// Writes size prefixed \a entity into \a output.
	template<typename TIo>
	void WriteEntity(TIo& output, const model::SizePrefixedEntity& entity) {
		output.write({ reinterpret_cast<const uint8_t*>(&entity), entity.Size });
	}

	/// Reads size prefixed entity from \a input.
	template<typename TEntity, typename TIo>
	std::unique_ptr<TEntity> ReadEntity(TIo& input) {
		auto entitySize = Read32(input);
		auto pEntity = utils::MakeUniqueWithSize<TEntity>(entitySize);
		input.read({ reinterpret_cast<uint8_t*>(pEntity.get()) + sizeof(uint32_t), entitySize - sizeof(uint32_t) });
		pEntity->Size = entitySize;
		return pEntity;
	}
}}

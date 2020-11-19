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

#include "Nemesis.h"
#include "Conversions.h"
#include "data/NemesisMemoryBlockStorage_data.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace test {

	GenerationHashSeed GetNemesisGenerationHashSeed() {
		constexpr auto Nemesis_Generation_Hash_Seed_String = "57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6";
		return utils::ParseByteArray<GenerationHashSeed>(Nemesis_Generation_Hash_Seed_String);
	}

	const model::Block& GetNemesisBlock() {
		return reinterpret_cast<const model::Block&>(MemoryBlockStorage_NemesisBlockData);
	}
}}

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

#include "Nemesis.h"
#include "Conversions.h"
#ifndef SIGNATURE_SCHEME_NIS1
#include "data/NemesisMemoryBlockStorage_data.h"
#else
#include "data/NemesisMemoryBlockStorage_data.nis1.h"
#endif

namespace catapult { namespace test {

	Hash256 GetNemesisGenerationHash() {
#ifdef SIGNATURE_SCHEME_NIS1
		constexpr auto Nemesis_Generation_Hash_String = "16ED3D69d3CA67132AACE4405AA122E5E041E58741A4364255B15201F5AAF6E4";
#else
		constexpr auto Nemesis_Generation_Hash_String = "57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6";
#endif

		return ToArray<Hash256_Size>(Nemesis_Generation_Hash_String);
	}

	const model::Block& GetNemesisBlock() {
		return reinterpret_cast<const model::Block&>(MemoryBlockStorage_NemesisBlockData);
	}
}}

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

#include "CacheSerializationTestUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/nodeps/Random.h"
#include <cstring>

namespace catapult { namespace test {

	bool CacheSerializationTestEntry::operator==(const CacheSerializationTestEntry& rhs) const {
		return Alpha == rhs.Alpha && Beta == rhs.Beta && Gamma == rhs.Gamma;
	}

	CacheSerializationTestEntry CacheSerializationTestEntryLoaderTraits::Load(io::InputStream& input) {
		CacheSerializationTestEntry entry;
		input.read({ reinterpret_cast<uint8_t*>(&entry), sizeof(CacheSerializationTestEntry) });
		return entry;
	}

	void CacheSerializationTestEntryLoaderTraits::LoadInto(const CacheSerializationTestEntry& entry, DestinationType& destination) {
		destination.push_back(entry);
	}

	std::vector<CacheSerializationTestEntry> GenerateRandomCacheSerializationTestEntries(size_t count) {
		return GenerateRandomDataVector<CacheSerializationTestEntry>(count);
	}

	std::vector<uint8_t> CopyCacheSerializationTestEntriesToStreamBuffer(const std::vector<CacheSerializationTestEntry>& entries) {
		auto numHeaderBytes = sizeof(uint64_t);
		uint64_t numBytes = numHeaderBytes + entries.size() * sizeof(CacheSerializationTestEntry);
		std::vector<uint8_t> buffer(numBytes);
		reinterpret_cast<uint64_t&>(buffer[0]) = entries.size();
		utils::memcpy_cond(buffer.data() + numHeaderBytes, entries.data(), numBytes - numHeaderBytes);
		return buffer;
	}
}}

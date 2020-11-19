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
#include "CacheChanges.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include <vector>

namespace catapult { namespace cache {

	/// Writes serialized cache \a changes into \a outputStream.
	template<typename TSerializer, typename TCacheDelta, typename TValue>
	void WriteCacheChanges(const SingleCacheChangesT<TCacheDelta, TValue>& changes, io::OutputStream& outputStream) {
		auto writeAllFrom = [&outputStream](const auto& source) {
			for (const auto* pValue : source)
				TSerializer::Save(*pValue, outputStream);
		};

		const auto& addedElements = changes.addedElements();
		const auto& removedElements = changes.removedElements();
		const auto& modifiedElements = changes.modifiedElements();

		io::Write64(outputStream, addedElements.size());
		io::Write64(outputStream, removedElements.size());
		io::Write64(outputStream, modifiedElements.size());

		writeAllFrom(addedElements);
		writeAllFrom(removedElements);
		writeAllFrom(modifiedElements);
	}

	/// Reads serialized cache \a changes from \a inputStream.
	template<typename TSerializer, typename TValue>
	void ReadCacheChanges(io::InputStream& inputStream, MemoryCacheChangesT<TValue>& changes) {
		auto readAllInto = [&inputStream](auto& dest, auto count) {
			for (auto i = 0u; i < count; ++i)
				dest.push_back(TSerializer::Load(inputStream));
		};

		auto numAdded = io::Read64(inputStream);
		auto numRemoved = io::Read64(inputStream);
		auto numCopied = io::Read64(inputStream);

		readAllInto(changes.Added, numAdded);
		readAllInto(changes.Removed, numRemoved);
		readAllInto(changes.Copied, numCopied);
	}
}}

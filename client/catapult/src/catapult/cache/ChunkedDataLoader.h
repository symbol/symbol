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
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/functions.h"

namespace catapult { namespace cache {

	/// Loads data from an input stream in chunks.
	template<typename TStorageTraits>
	class ChunkedDataLoader {
	private:
		using LoaderFunc = consumer<io::InputStream&, typename TStorageTraits::DestinationType&>;

		enum class LoaderType { Basic, Stateful, CacheDependent };
		using BasicLoaderFlag = std::integral_constant<LoaderType, LoaderType::Basic>;
		using StatefulLoaderFlag = std::integral_constant<LoaderType, LoaderType::Stateful>;

	public:
		/// Creates a chunked loader around \a input.
		explicit ChunkedDataLoader(io::InputStream& input)
				: m_input(input)
				, m_loader(CreateLoader(LoadStateAccessor<TStorageTraits>())) {
			m_numRemainingEntries = io::Read64(input);
		}

	public:
		/// Returns \c true if there are more entries in the input.
		bool hasNext() const {
			return 0 != m_numRemainingEntries;
		}

		/// Loads the next data chunk of at most \a numRequestedEntries into \a destination.
		void next(uint64_t numRequestedEntries, typename TStorageTraits::DestinationType& destination) {
			numRequestedEntries = std::min(numRequestedEntries, m_numRemainingEntries);
			m_numRemainingEntries -= numRequestedEntries;
			while (numRequestedEntries--)
				m_loader(m_input, destination);
		}

	private:
		template<typename T, typename = void>
		struct LoadStateAccessor : BasicLoaderFlag
		{};

		template<typename T>
		struct LoadStateAccessor<T, typename utils::traits::enable_if_type<typename T::LoadStateType>::type> : StatefulLoaderFlag
		{};

	private:
		static LoaderFunc CreateLoader(BasicLoaderFlag) {
			return TStorageTraits::LoadInto;
		}

		static LoaderFunc CreateLoader(StatefulLoaderFlag) {
			return [state = typename TStorageTraits::LoadStateType()](auto& input, auto& destination) mutable {
				return TStorageTraits::LoadInto(input, destination, state);
			};
		}

	private:
		io::InputStream& m_input;
		uint64_t m_numRemainingEntries;
		LoaderFunc m_loader;
	};
}}

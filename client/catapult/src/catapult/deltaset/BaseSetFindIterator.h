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
#include <utility>

namespace catapult { namespace deltaset {

	namespace detail {
		/// Iterator that represents a find result from a base set that supports one type of container iterator.
		/// \note This is always used for BaseSet and is used for BaseSetDelta when storage and memory sets have identical iterators.
		template<typename TFindTraits, typename TSetTraits, typename TStorageIterator, typename TFindResult>
		class BaseSetSingleIteratorWrapper {
		public:
			/// Creates an unset iterator.
			BaseSetSingleIteratorWrapper() : m_isSet(false)
			{}

			/// Creates an iterator around a storage iterator (\a storageIter).
			explicit BaseSetSingleIteratorWrapper(TStorageIterator&& storageIter)
					: m_isSet(true)
					, m_storageIter(std::move(storageIter))
			{}

		public:
			/// Gets the underlying value.
			TFindResult get() const {
				return m_isSet ? TFindTraits::ToResult(TSetTraits::ToValue(*m_storageIter)) : nullptr;
			}

		private:
			bool m_isSet;
			TStorageIterator m_storageIter;
		};

		/// Iterator that represents a find result from a base set that supports two types of container iterators.
		/// \note This is used for BaseSetDelta when storage and memory sets have different iterators.
		template<typename TFindTraits, typename TSetTraits, typename TStorageIterator, typename TMemoryIterator, typename TFindResult>
		class BaseSetDualIteratorWrapper {
		public:
			/// Creates an unset iterator.
			BaseSetDualIteratorWrapper() : m_iteratorType(IteratorType::Unset)
			{}

			/// Creates an iterator around a storage iterator (\a storageIter).
			explicit BaseSetDualIteratorWrapper(TStorageIterator&& storageIter)
					: m_iteratorType(IteratorType::Storage)
					, m_storageIter(std::move(storageIter))
			{}

			/// Creates an iterator around a memory iterator (\a memoryIter).
			explicit BaseSetDualIteratorWrapper(TMemoryIterator&& memoryIter)
					: m_iteratorType(IteratorType::Memory)
					, m_memoryIter(std::move(memoryIter))
			{}

		public:
			/// Gets the underlying value.
			TFindResult get() const {
				switch (m_iteratorType) {
				case IteratorType::Storage:
					return TFindTraits::ToResult(TSetTraits::ToValue(*m_storageIter));
				case IteratorType::Memory:
					return TFindTraits::ToResult(TSetTraits::ToValue(*m_memoryIter));
				default:
					return nullptr;
				}
			}

		private:
			enum class IteratorType { Unset, Storage, Memory };

		private:
			IteratorType m_iteratorType;
			TStorageIterator m_storageIter;
			TMemoryIterator m_memoryIter;
		};

		/// Iterator that represents a find result from a base set that supports either one or two types of container iterators.
		template<typename TFindTraits, typename TSetTraits, typename TStorageIterator, typename TMemoryIterator, typename TFindResult>
		using BaseSetConditionalIteratorWrapper = std::conditional_t<
			std::is_same_v<TStorageIterator, TMemoryIterator>,
			BaseSetSingleIteratorWrapper<TFindTraits, TSetTraits, TStorageIterator, TFindResult>,
			BaseSetDualIteratorWrapper<TFindTraits, TSetTraits, TStorageIterator, TMemoryIterator, TFindResult>
		>;
	}

	/// Iterator that returns a find result from a base set.
	template<typename TFindTraits, typename TSetTraits>
	using BaseSetFindIterator = detail::BaseSetSingleIteratorWrapper<
		TFindTraits,
		TSetTraits,
		typename TSetTraits::SetType::const_iterator,
		typename TFindTraits::ConstResultType
	>;

	/// Iterator that returns a find result from a base set delta.
	template<typename TFindTraits, typename TSetTraits>
	using BaseSetDeltaFindIterator = detail::BaseSetSingleIteratorWrapper<
		TFindTraits,
		TSetTraits,
		typename TSetTraits::MemorySetType::iterator,
		typename TFindTraits::ResultType
	>;

	/// Iterator that returns a find (const) result from a base set delta.
	template<typename TFindTraits, typename TSetTraits>
	using BaseSetDeltaFindConstIterator = detail::BaseSetConditionalIteratorWrapper<
		TFindTraits,
		TSetTraits,
		typename TSetTraits::SetType::const_iterator,
		typename TSetTraits::MemorySetType::const_iterator,
		typename TFindTraits::ConstResultType
	>;
}}

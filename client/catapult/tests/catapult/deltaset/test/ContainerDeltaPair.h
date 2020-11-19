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
#include <memory>

namespace catapult { namespace test {

	/// Pair composed of a container and a container delta.
	template<typename TTraits>
	class ContainerDeltaPair {
	public:
		using Type = typename TTraits::Type;
		using DeltaType = typename TTraits::DeltaType;

	public:
		/// Wraps \a pContainer and \a pContainerDelta.
		ContainerDeltaPair(const std::shared_ptr<Type>& pContainer, const std::shared_ptr<DeltaType>& pContainerDelta)
				: m_pContainer(pContainer)
				, m_pContainerDelta(pContainerDelta)
		{}

	public:
		/// Gets a pointer to the underlying delta.
		auto operator->() const {
			return m_pContainerDelta.get();
		}

		/// Gets a reference to the underlying delta.
		auto& operator*() const {
			return *m_pContainerDelta;
		}

	public:
		/// Gets the size of the original (base) container.
		size_t originalSize() const {
			return m_pContainer->size();
		}

	public:
		/// Commits delta changes to the underlying container.
		void commit() {
			m_pContainer->commit();
		}

	private:
		std::shared_ptr<Type> m_pContainer;
		std::shared_ptr<DeltaType> m_pContainerDelta;
	};
}}

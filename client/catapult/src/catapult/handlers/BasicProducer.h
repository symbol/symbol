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

namespace catapult { namespace handlers {

	/// Base class for producers.
	template<typename TContainer>
	class BasicProducer {
	public:
		/// Creates a producer around \a container.
		explicit BasicProducer(const TContainer& container)
				: m_container(container)
				, m_iter(m_container.cbegin())
		{}

	protected:
		/// Produces the next entity and calls \a convert for any required adaptation.
		template<typename TConverter>
		auto next(TConverter convert) {
			return m_container.cend() == m_iter ? nullptr : convert(*m_iter++);
		}

	private:
		const TContainer& m_container;
		decltype(TContainer().cbegin()) m_iter;
	};
}}

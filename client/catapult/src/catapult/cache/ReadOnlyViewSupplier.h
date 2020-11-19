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

namespace catapult { namespace cache {

	/// Decorates a view and supplies a read-only view.
	template<typename TView>
	class ReadOnlyViewSupplier : public TView {
	public:
		/// Creates a read-only view supplier around \a args.
		template<typename... TArgs>
		ReadOnlyViewSupplier(TArgs&&... args)
				: TView(std::forward<TArgs>(args)...)
				, m_readOnlyView(*this)
		{}

		/// Move constructs a read-only view supplier from \a rhs.
		/// \note Default move constructor will not work because pointer in m_readOnlyView needs to be updated.
		ReadOnlyViewSupplier(ReadOnlyViewSupplier&& rhs)
				: TView(std::move(rhs))
				, m_readOnlyView(*this)
		{}

	public:
		/// Gets a read-only view of this view.
		const typename TView::ReadOnlyView& asReadOnly() const {
			return m_readOnlyView;
		}

	private:
		typename TView::ReadOnlyView m_readOnlyView;
	};
}}

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

namespace catapult { namespace utils {

	/// Decorates a handler by additionally capturing a shared pointer to an owning object,
	/// which extends that object's lifetime.
	template<typename THandler>
	class WrappedWithOwnerDecorator {
	public:
		/// Wraps \a handler and keeps \a pOwner alive.
		template<typename TOwner>
		WrappedWithOwnerDecorator(const std::shared_ptr<TOwner>& pOwner, THandler handler)
				: m_pOwner(pOwner)
				, m_handler(std::move(handler))
		{}

	public:
		/// Forwards \a args to the wrapped handler.
		template<typename... TArgs>
		auto operator()(TArgs&& ...args) const {
			return m_handler(std::forward<TArgs>(args)...);
		}

		/// Reference to the owning pointer.
		const std::shared_ptr<const void>& owner() const {
			return m_pOwner;
		}

	protected:
		std::shared_ptr<const void> m_pOwner; // owner must be destroyed last in case handler holds raw reference(s) to it
		THandler m_handler;
	};

	/// Decorates a handler by additionally capturing a shared pointer to an owning object,
	/// which extends that object's lifetime. This decorator supports reset.
	template<typename THandler>
	class ResettableWrappedWithOwnerDecorator : public WrappedWithOwnerDecorator<THandler> {
	public:
		/// Wraps \a handler and keeps \a pOwner alive.
		template<typename TOwner>
		ResettableWrappedWithOwnerDecorator(const std::shared_ptr<TOwner>& pOwner, THandler handler)
				: WrappedWithOwnerDecorator<THandler>(pOwner, std::move(handler))
		{}

	public:
		/// Releases all resources associated with this object.
		void reset() {
			using BaseType = WrappedWithOwnerDecorator<THandler>;
			auto empty = decltype(BaseType::m_handler)();
			BaseType::m_handler.swap(empty);
			BaseType::m_pOwner.reset();
		}
	};
}}

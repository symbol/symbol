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
#include "catapult/utils/WrappedWithOwnerDecorator.h"
#include <boost/asio.hpp>

namespace catapult { namespace thread {

	/// Wraps a strand and automatically augments handlers to extend the lifetime of an owning object.
	template<typename TOwner>
	class StrandOwnerLifetimeExtender {
	public:
		/// Creates a strand owner lifetime extender around \a strand.
		explicit StrandOwnerLifetimeExtender(boost::asio::io_context::strand& strand) : m_strand(strand)
		{}

	public:
		/// Wraps \a handler and attaches \a pOwner to it.
		template<typename THandler>
		auto wrap(const std::shared_ptr<TOwner>& pOwner, THandler handler) {
			// when wrap is called the returned callback needs to extend the lifetime of pOwner
			utils::WrappedWithOwnerDecorator<THandler> wrappedHandler(pOwner, std::move(handler));
			return m_strand.wrap(wrappedHandler);
		}

	public:
		/// Attaches \a pOwner to \a handler and posts it to the strand.
		template<typename THandler>
		void post(const std::shared_ptr<TOwner>& pOwner, THandler handler) {
			// ensure all handlers extend the lifetime of pOwner and post to a strand
			boost::asio::post(m_strand, [pOwner, handler]() {
				handler(pOwner);
			});
		}

	private:
		boost::asio::io_context::strand& m_strand;
	};
}}

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

#include "Resolvable.h"
#include "ResolverContext.h"
#include "catapult/exceptions.h"

namespace catapult { namespace model {

	namespace {
		UnresolvedAddress Unresolve(const Address& address) {
			return address.copyTo<UnresolvedAddress>();
		}

		UnresolvedMosaicId Unresolve(MosaicId mosaicId) {
			return UnresolvedMosaicId(mosaicId.unwrap());
		}
	}

	template<typename TUnresolved, typename TResolved>
	Resolvable<TUnresolved, TResolved>::Resolvable() : m_type(Type::Resolved)
	{}

	template<typename TUnresolved, typename TResolved>
	Resolvable<TUnresolved, TResolved>::Resolvable(const TUnresolved& unresolved)
			: m_unresolved(unresolved)
			, m_type(Type::Unresolved)
	{}

	template<typename TUnresolved, typename TResolved>
	Resolvable<TUnresolved, TResolved>::Resolvable(const TResolved& resolved)
			: m_resolved(resolved)
			, m_type(Type::Resolved)
	{}

	template<typename TUnresolved, typename TResolved>
	bool Resolvable<TUnresolved, TResolved>::isResolved() const {
		return Type::Resolved == m_type;
	}

	template<typename TUnresolved, typename TResolved>
	TUnresolved Resolvable<TUnresolved, TResolved>::unresolved() const {
		return isResolved() ? Unresolve(m_resolved) : m_unresolved;
	}

	template<typename TUnresolved, typename TResolved>
	TResolved Resolvable<TUnresolved, TResolved>::resolved() const {
		if (!isResolved())
			CATAPULT_THROW_INVALID_ARGUMENT("underlying value is not resolved and requires ResolverContext");

		return m_resolved;
	}

	template<typename TUnresolved, typename TResolved>
	TResolved Resolvable<TUnresolved, TResolved>::resolved(const ResolverContext& resolvers) const {
		return isResolved() ? m_resolved : resolvers.resolve(m_unresolved);
	}

	template class Resolvable<UnresolvedAddress, Address>;
	template class Resolvable<UnresolvedMosaicId, MosaicId>;
}}

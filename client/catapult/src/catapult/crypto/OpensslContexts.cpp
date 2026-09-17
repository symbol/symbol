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

#include "OpensslContexts.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/evp.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

	// region OpensslDigestContext

	OpensslDigestContext::OpensslDigestContext() : m_pContext(EVP_MD_CTX_new()) {
		if (!m_pContext)
			throw std::bad_alloc();
	}

	OpensslDigestContext::~OpensslDigestContext() {
		EVP_MD_CTX_free(m_pContext);
	}

	OpensslDigestContext::context_type* OpensslDigestContext::get() {
		return m_pContext;
	}

	// endregion

	// region OpensslCipherContext

	OpensslCipherContext::OpensslCipherContext() : m_pContext(EVP_CIPHER_CTX_new()) {
		if (!m_pContext)
			throw std::bad_alloc();
	}

	OpensslCipherContext::~OpensslCipherContext() {
		EVP_CIPHER_CTX_free(m_pContext);
	}

	OpensslCipherContext::context_type* OpensslCipherContext::get() {
		return m_pContext;
	}

	// endregion
}}

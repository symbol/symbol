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
#include "catapult/exceptions.h"
#include "catapult/preprocessor.h"
#include "catapult/utils/NonCopyable.h"

struct evp_cipher_ctx_st;
struct MAY_ALIAS evp_md_ctx_st;

namespace catapult { namespace crypto {

	// region OpensslDigestContext

	/// Wrapper for openssl digest context.
	class alignas(32) OpensslDigestContext {
	private:
		using context_type = evp_md_ctx_st;

	public:
		/// Creates a new context.
		OpensslDigestContext();

		/// Destroys the context.
		~OpensslDigestContext();

	public:
		/// Dispatches an openssl digest call to \a func with \a args.
		template<typename TFunc, typename... TArgs>
		void dispatch(TFunc func, TArgs&&... args) {
			auto result = func(get(), std::forward<TArgs>(args)...);

			if (!result)
				CATAPULT_THROW_RUNTIME_ERROR_1("openssl digest operation failed ", result);
		}

	private:
		context_type* get();
		void reset();

	private:
		uint8_t m_buffer[256];
	};

	// endregion

	// region OpensslCipherContext

	/// Wrapper for openssl cipher context.
	class OpensslCipherContext : public utils::NonCopyable {
	private:
		using context_type = evp_cipher_ctx_st;

	public:
		/// Creates a new context.
		OpensslCipherContext();

		/// Destroys the context.
		~OpensslCipherContext();

	public:
		/// Dispatches an openssl cipher call to \a func with \a args.
		template<typename TFunc, typename... TArgs>
		void dispatch(TFunc func, TArgs&&... args) {
			auto result = func(get(), std::forward<TArgs>(args)...);

			if (!result)
				CATAPULT_THROW_RUNTIME_ERROR_1("openssl cipher operation failed ", result);
		}

		/// Tries to dispatch an openssl cipher call to \a func with \a args.
		template<typename TFunc, typename... TArgs>
		bool tryDispatch(TFunc func, TArgs&&... args) {
			return !!func(get(), std::forward<TArgs>(args)...);
		}

	private:
		context_type* get();

	private:
		// Switch to using EVP_CIPHER_CTX_new / EVP_CIPHER_CTX_free since the EVP_CIPHER_CTX_reset() does not work since
		// it exit early OpenSSL treats this struct as an opaque type, so decided to not update the value directly
		context_type* m_pContext;
	};

	// endregion
}}

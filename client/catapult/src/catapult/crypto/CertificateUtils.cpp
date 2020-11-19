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

#include "CertificateUtils.h"
#include <cstring>
#include <memory>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

	namespace {
		// region MemoryStream

		class MemoryStream {
		public:
			MemoryStream() : m_pImpl(std::shared_ptr<BIO>(BIO_new(BIO_s_mem()), BIO_free)) {
				if (!m_pImpl)
					throw std::bad_alloc();
			}

		public:
			bool printEx(const X509_NAME& name) {
				return -1 != X509_NAME_print_ex(m_pImpl.get(), &name, 0, XN_FLAG_RFC2253);
			}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

			std::string toString() {
				char* pRawData;
				auto dataSize = BIO_get_mem_data(m_pImpl.get(), &pRawData);

				std::string result;
				if (pRawData) {
					result.resize(static_cast<size_t>(dataSize));
					std::memcpy(result.data(), pRawData, result.size());
				}

				return result;
			}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

		private:
			std::shared_ptr<BIO> m_pImpl;
		};

		// endregion

		bool ExtractOneLine(const X509_NAME& name, std::string& result) {
			MemoryStream memoryStream;
			if (!memoryStream.printEx(name))
				return false;

			result = memoryStream.toString();
			return true;
		}
	}

	bool TryParseCertificate(const X509& certificate, CertificateInfo& certificateInfo) {
		const auto* pSubject = X509_get_subject_name(&certificate);
		if (!ExtractOneLine(*pSubject, certificateInfo.Subject))
			return false;

		auto nullTerminatorIndex = certificateInfo.Subject.find_last_not_of('\0');
		certificateInfo.Subject.resize(std::string::npos == nullTerminatorIndex ? 0 : nullTerminatorIndex + 1);

		const auto* pCertificatePublicKey = X509_get0_pubkey(&certificate);
		if (!pCertificatePublicKey)
			return false;

		if (EVP_PKEY_ED25519 != EVP_PKEY_id(pCertificatePublicKey))
			return false;

		auto keySize = certificateInfo.PublicKey.size();
		return EVP_PKEY_get_raw_public_key(pCertificatePublicKey, certificateInfo.PublicKey.data(), &keySize) && Key::Size == keySize;
	}

	namespace {
		template<typename TObject, typename TFunc, typename... TArgs>
		bool Dispatch(const std::shared_ptr<TObject>& pObject, TFunc func, TArgs&&... args) {
			if (!pObject)
				throw std::bad_alloc();

			return func(pObject.get(), std::forward<TArgs>(args)...);
		}
	}

	bool VerifySelfSigned(X509& certificate) {
		auto pCertificateStore = std::shared_ptr<X509_STORE>(X509_STORE_new(), X509_STORE_free);
		if (!Dispatch(pCertificateStore, X509_STORE_add_cert, &certificate))
			return false;

		auto pCertificateStoreContext = std::shared_ptr<X509_STORE_CTX>(X509_STORE_CTX_new(), X509_STORE_CTX_free);
		if (!Dispatch(pCertificateStoreContext, X509_STORE_CTX_init, pCertificateStore.get(), &certificate, nullptr))
			return false;

		X509_STORE_CTX_set_flags(pCertificateStoreContext.get(), X509_V_FLAG_CHECK_SS_SIGNATURE);
		return 1 == X509_verify_cert(pCertificateStoreContext.get());
	}
}}

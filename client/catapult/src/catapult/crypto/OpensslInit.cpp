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

#include "OpensslInit.h"
#include "catapult/exceptions.h"
#include <openssl/provider.h>

namespace catapult { namespace crypto {

	namespace {
		struct OpensslContext {
			std::shared_ptr<OSSL_PROVIDER> pDefaultProvider;
			std::shared_ptr<OSSL_PROVIDER> pLegacyProvider;
		};
	}

	std::shared_ptr<void> SetupOpensslCryptoFunctions() {
		auto pContext = std::make_shared<OpensslContext>();

		pContext->pDefaultProvider.reset(OSSL_PROVIDER_load(nullptr, "default"), OSSL_PROVIDER_unload);
		pContext->pLegacyProvider.reset(OSSL_PROVIDER_load(nullptr, "legacy"), OSSL_PROVIDER_unload);

		if (!pContext->pDefaultProvider || !pContext->pLegacyProvider)
			CATAPULT_THROW_RUNTIME_ERROR("unable to load required OpenSSL crypto providers");

		return pContext;
	}
}}

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

#include "src/handlers/NamespaceDiagnosticHandlers.h"
#include "tests/test/cache/CacheEntryInfosHandlerTestTraits.h"
#include "tests/test/plugins/BatchHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	namespace {
		// region mosaic infos

		struct MosaicInfosTraits : public test::CacheEntryInfoHandlerTestTraits<MosaicId, ionet::PacketType::Mosaic_Infos> {
		public:
			static constexpr auto Valid_Request_Payload_Size = sizeof(MosaicId);

		public:
			template<typename TAction>
			static void RegisterHandler(ionet::ServerPacketHandlers& handlers, TAction action) {
				RegisterMosaicInfosHandler(handlers, test::BatchHandlerSupplierActionToProducer<ResponseType>(action));
			}
		};

		// endregion

		// region namespace infos

		struct NamespaceInfosTraits : public test::CacheEntryInfoHandlerTestTraits<NamespaceId, ionet::PacketType::Namespace_Infos> {
		public:
			static constexpr auto Valid_Request_Payload_Size = sizeof(NamespaceId);

		public:
			template<typename TAction>
			static void RegisterHandler(ionet::ServerPacketHandlers& handlers, TAction action) {
				RegisterNamespaceInfosHandler(handlers, test::BatchHandlerSupplierActionToProducer<ResponseType>(action));
			}
		};

		// endregion
	}

	DEFINE_BATCH_HANDLER_TESTS(NamespaceDiagnosticHandlersTests, MosaicInfos)
	DEFINE_BATCH_HANDLER_TESTS(NamespaceDiagnosticHandlersTests, NamespaceInfos)
}}

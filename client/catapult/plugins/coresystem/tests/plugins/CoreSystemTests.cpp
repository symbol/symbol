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

#include "src/plugins/CoreSystem.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct CoreSystemTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto manager = test::CreatePluginManager();
				RegisterCoreSystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Voting_Key_Link, model::Entity_Type_Vrf_Key_Link };
			}

			static std::vector<std::string> GetCacheNames() {
				return { "AccountStateCache", "BlockStatisticCache" };
			}

			static std::vector<ionet::PacketType> GetNonDiagnosticPacketTypes() {
				return { ionet::PacketType::Account_State_Path };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Account_Infos };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "ACNTST C", "ACNTST C HVA", "BLKDIF C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return {
					"ZeroAddressValidator",
					"ZeroPublicKeyValidator",
					"MaxTransactionsValidator",
					"NetworkValidator",
					"EntityVersionValidator",
					"TransactionFeeValidator",
					"KeyLinkActionValidator",
					"ZeroInternalPaddingValidator",
					"VotingKeyLinkRangeValidator"
				};
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {
					"AddressValidator",
					"PublicKeyValidator",
					"DeadlineValidator",
					"NemesisSinkValidator",
					"EligibleHarvesterValidator",
					"BalanceDebitValidator",
					"BalanceTransferValidator",

					// key link transactions
					"VrfKeyLinkValidator",
					"VotingMultiKeyLinkValidator"
				};
			}

			static std::vector<std::string> GetObserverNames() {
				auto names = GetPermanentObserverNames();

				// transient observers
				names.push_back("RecalculateImportancesObserver");
				names.push_back("HighValueAccountRollbackObserver");
				names.push_back("BlockStatisticObserver");
				return names;
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return {
					"SourceChangeObserver",
					"AccountAddressObserver",
					"AccountPublicKeyObserver",
					"BalanceDebitObserver",
					"BalanceTransferObserver",
					"BeneficiaryObserver",
					"TransactionFeeActivityObserver",
					"HarvestFeeObserver",
					"TotalTransactionsObserver",
					"HighValueAccountCommitObserver",

					// key link transactions
					"VrfKeyLinkObserver",
					"VotingMultiKeyLinkObserver"
				};
			}
		};
	}

	DEFINE_PLUGIN_TESTS(CoreSystemTests, CoreSystemTraits)
}}

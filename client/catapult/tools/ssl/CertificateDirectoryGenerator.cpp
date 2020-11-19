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

#include "CertificateDirectoryGenerator.h"
#include "CertificateUtils.h"
#include "tools/ToolKeys.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/RawFile.h"
#include "catapult/exceptions.h"
#include <filesystem>

namespace catapult { namespace tools { namespace ssl {

	namespace {
		void SaveToFile(const std::string& directory, const std::string& filename, const std::string& buffer) {
			io::RawFile dataFile((std::filesystem::path(directory) / filename).generic_string(), io::OpenMode::Read_Write);
			dataFile.write({ reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size() });
		}

		auto CreateCertificateBuilder(const Key& publicKey) {
			CertificateBuilder builder;
			builder.setSubject("XD", "CA", "Ca certificate");
			builder.setIssuer("XD", "CA", "Ca certificate");
			builder.setPublicKey(publicKey);
			return builder;
		}

		enum class CertificateFlags {
			None,
			Expired,
			Malformed_Signature
		};

		auto ScenarioIdToFlags(ScenarioId scenarioId) {
			switch (scenarioId) {
				case ScenarioId::Expired_Ca_Certificate:
				case ScenarioId::Expired_Node_Certificate:
					return CertificateFlags::Expired;
				case ScenarioId::Malformed_Ca_Certificate_Signature:
				case ScenarioId::Malformed_Node_Certificate_Signature:
					return CertificateFlags::Malformed_Signature;
				default:
					return CertificateFlags::None;
			}
		}

		auto BuildCertificate(CertificateBuilder& builder, CertificateFlags flags, const crypto::KeyPair& signingKeyPair) {
			if (CertificateFlags::Expired == flags)
				builder.setValidity(-1 * 60 * 60 * 24 * 10, -1);

			auto pCertificate = builder.buildAndSign(signingKeyPair);
			if (CertificateFlags::Malformed_Signature != flags)
				return pCertificate;

			// change validity to 364 days, this will invalidate signature
			builder.setValidity(0, 364 * 24 * 60 * 60);
			return builder.build();
		}

		std::vector<std::shared_ptr<x509_st>> CreateFullCertificateChain(
				const crypto::KeyPair& caKeyPair,
				const crypto::KeyPair& nodeKeyPair,
				ScenarioId scenarioId) {
			// CA cert
			auto caCertificateBuilder = CreateCertificateBuilder(caKeyPair.publicKey());
			auto caFlags = ScenarioIdToFlags(scenarioId);
			auto pCaCertificate = BuildCertificate(caCertificateBuilder, caFlags, caKeyPair);

			// node cert
			auto nodeCertificateBuilder = CreateCertificateBuilder(nodeKeyPair.publicKey());
			nodeCertificateBuilder.setSubject("US", "Symbol", "nijuichi");
			auto nodeFlags = ScenarioIdToFlags(scenarioId);
			auto pNodeCertificate = BuildCertificate(nodeCertificateBuilder, nodeFlags, caKeyPair);

			if (ScenarioId::Three_Level_Certificate_Chain == scenarioId) {
				// `rejecting certificate chain with size 3`
				auto middleKeyPair = GenerateRandomKeyPair();
				auto middleCertificateBuilder = CreateCertificateBuilder(middleKeyPair.publicKey());
				middleCertificateBuilder.setSubject("MD", "MD", "Middle level certificate");
				auto pMiddleLevel = middleCertificateBuilder.buildAndSign(caKeyPair);

				// fix node certificate issuer and regenerate node certificate
				nodeCertificateBuilder.setIssuer("MD", "MD", "Middle level certificate");
				pNodeCertificate = nodeCertificateBuilder.buildAndSign(middleKeyPair);

				return { pNodeCertificate, pMiddleLevel, pCaCertificate };
			} else if (ScenarioId::Single_Self_Signed_Ca_Certificate == scenarioId) {
				// `rejecting certificate chain with size 1`
				pNodeCertificate = nodeCertificateBuilder.buildAndSign(nodeKeyPair);

				return { pNodeCertificate };
			}

			return { pNodeCertificate, pCaCertificate };
		}
	}

	void GenerateCertificateDirectory(crypto::KeyPair&& caKeyPair, const std::string& certificateDirectory, ScenarioId scenarioId) {
		if (std::filesystem::exists(certificateDirectory))
			std::filesystem::remove_all(certificateDirectory);

		CATAPULT_LOG(info) << "generating new certificate directory: " << certificateDirectory;
		config::CatapultDirectory(certificateDirectory).createAll();

		auto nodeKeyPair = GenerateRandomKeyPair();

		// note: right now we're not rejecting this
		if (ScenarioId::Two_Level_Certificate_Chain_With_Same_Key == scenarioId)
			nodeKeyPair = CopyKeyPair(caKeyPair);

		auto certificates = CreateFullCertificateChain(caKeyPair, nodeKeyPair, scenarioId);
		auto fullCertificateChain = CreateFullCertificateChainPem(certificates);
		SaveToFile(certificateDirectory, "node.key.pem", CreatePrivateKeyPem(nodeKeyPair));
		SaveToFile(certificateDirectory, "node.full.crt.pem", fullCertificateChain);
	}
}}}
